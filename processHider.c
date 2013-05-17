#include "processHider.h"

// From Linux Kernel  3.2.0-40-generic #64-Ubuntu  with modifications
void do_exit(long code) {
	struct task_struct *tsk = current;
	int group_dead;

	profile_task_exit(tsk);

	WARN_ON(blk_needs_flush_plug(tsk));

	if (unlikely(in_interrupt()))
		panic("Aiee, killing interrupt handler!");
	if (unlikely(!tsk->pid))
		panic("Attempted to kill the idle task!");

	/*
	 * If do_exit is called because this processes oopsed, it's possible
	 * that get_fs() was left as KERNEL_DS, so reset it to USER_DS before
	 * continuing. Amongst other possible reasons, this is to prevent
	 * mm_release()->clear_child_tid() from writing to a user-controlled
	 * kernel address.
	 */
	set_fs(USER_DS);

	ptrace_event(PTRACE_EVENT_EXIT, code);

	validate_creds_for_do_exit(tsk);

	/*
	 * We're taking recursive faults here in do_exit. Safest is to just
	 * leave this task alone and wait for reboot.
	 */
	if (unlikely(tsk->flags & PF_EXITING)) {
		printk(KERN_ALERT
			"Fixing recursive fault but reboot is needed!\n");
		/*
		 * We can do this unlocked here. The futex code uses
		 * this flag just to verify whether the pi state
		 * cleanup has been done or not. In the worst case it
		 * loops once more. We pretend that the cleanup was
		 * done as there is no way to return. Either the
		 * OWNER_DIED bit is set by now or we push the blocked
		 * task into the wait for ever nirwana as well.
		 */
		tsk->flags |= PF_EXITPIDONE;
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule();
	}

	exit_irq_thread();

	exit_signals(tsk);  /* sets PF_EXITING */
	/*
	 * tsk->flags are checked in the futex code to protect against
	 * an exiting task cleaning up the robust pi futexes.
	 */
	smp_mb();
	raw_spin_unlock_wait(&tsk->pi_lock);

	if (unlikely(in_atomic()))
		printk(KERN_INFO "note: %s[%d] exited with preempt_count %d\n",
				current->comm, task_pid_nr(current),
				preempt_count());

	acct_update_integrals(tsk);
	/* sync mm's RSS info before statistics gathering */
	if (tsk->mm)
		sync_mm_rss(tsk, tsk->mm);
	group_dead = atomic_dec_and_test(&tsk->signal->live);
	if (group_dead) {
		hrtimer_cancel(&tsk->signal->real_timer);
		exit_itimers(tsk->signal);
		if (tsk->mm)
			setmax_mm_hiwater_rss(&tsk->signal->maxrss, tsk->mm);
	}
	acct_collect(code, group_dead);
	if (group_dead)
		tty_audit_exit();
	if (unlikely(tsk->audit_context))
		audit_free(tsk);

	tsk->exit_code = code;
	taskstats_exit(tsk, group_dead);

	exit_mm(tsk);

	if (group_dead)
		acct_process();
	trace_sched_process_exit(tsk);

	exit_sem(tsk);
	exit_shm(tsk);
	exit_files(tsk);
	exit_fs(tsk);
	check_stack_usage();
	exit_thread();

	/*
	 * Flush inherited counters to the parent - before the parent
	 * gets woken up by child-exit notifications.
	 *
	 * because of cgroup mode, must be called before cgroup_exit()
	 */
	perf_event_exit_task(tsk);

	cgroup_exit(tsk, 1);

	if (group_dead)
		disassociate_ctty(1);

	module_put(task_thread_info(tsk)->exec_domain->module);

	proc_exit_connector(tsk);

	/*
	 * FIXME: do that only when needed, using sched_exit tracepoint
	 */
	ptrace_put_breakpoints(tsk);

	exit_notify(tsk, group_dead);
#ifdef CONFIG_NUMA
	task_lock(tsk);
	mpol_put(tsk->mempolicy);
	tsk->mempolicy = NULL;
	task_unlock(tsk);
#endif
#ifdef CONFIG_FUTEX
	if (unlikely(current->pi_state_cache))
		kfree(current->pi_state_cache);
#endif
	/*
	 * Make sure we are holding no locks:
	 */
	debug_check_no_locks_held(tsk);
	/*
	 * We can do this unlocked here. The futex code uses this flag
	 * just to verify whether the pi state cleanup has been done
	 * or not. In the worst case it loops once more.
	 */
	tsk->flags |= PF_EXITPIDONE;

	if (tsk->io_context)
		exit_io_context(tsk);

	if (tsk->splice_pipe)
		__free_pipe_info(tsk->splice_pipe);

	validate_creds_for_do_exit(tsk);

	preempt_disable();
	exit_rcu();

	/*
	 * The setting of TASK_RUNNING by try_to_wake_up() may be delayed
	 * when the following two conditions become true.
	 *   - There is race condition of mmap_sem (It is acquired by
	 *     exit_mm()), and
	 *   - SMI occurs before setting TASK_RUNINNG.
	 *     (or hypervisor of virtual machine switches to other guest)
	 *  As a result, we may become TASK_RUNNING after becoming TASK_DEAD
	 *
	 * To avoid it, we have to wait for releasing tsk->pi_lock which
	 * is held by try_to_wake_up()
	 */
	smp_mb();
	raw_spin_unlock_wait(&tsk->pi_lock);

	/* causes final put_task_struct in finish_task_switch(). */
	tsk->state = TASK_DEAD;
	schedule();
	BUG();
	
	/* Avoid "noreturn function does return".  */
	for (;;)
		cpu_relax();	/* For when BUG is null */
}

