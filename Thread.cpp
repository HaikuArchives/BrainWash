// Thread.cpp

#include "Thread.h"

// Thread

// constructor
Thread::Thread(const char *name, int32 priority)
{
	id = spawn_thread(run_thread, name, priority, this);
	go = false;
}

// destructor
Thread::~Thread()
{
	// the emergency brake:
	// we've already left the destructors of derived classes, so
	// if the thread is still running the thread function might misbehave
	if (id >= B_NO_ERROR)
		Die();
}

// Exit
void Thread::Exit(status_t ret_val)
{
	go = false;
	id = B_ERROR;
	exit_thread(ret_val);
}

// Kill
status_t Thread::Kill()
{
	status_t result = B_BAD_THREAD_ID;
	if (id >= B_NO_ERROR)
	{
		result = kill_thread(id);
		go = false;
		id = B_ERROR;
	}
	return result;
};

// Die
void Thread::Die()
{
	if (id >= B_NO_ERROR)
	{
		status_t exitval;
		if (go)
		{
			go = false;
			Terminate();
			WaitFor(&exitval);
		}
		else
			Kill();
	}
}

// run_thread
int32 Thread::run_thread(void *data)
{
	Thread *thread = reinterpret_cast<Thread *>(data);
	thread->go = true;
	int32 result = thread->ThreadFunction();
	thread->id = B_ERROR;
	thread->go = false;
	return result;
}


// CmdThread

// ThreadFunction
int32 CmdThread::ThreadFunction()
{
	int32 result = CmdThreadFunction();
	// clean up the thread's message buffer (the unprocessed Commands)
	Command *cmd;
	while (HasData() && (cmd = ReceiveCommand()))
		CommandProcessed(cmd);
	return result;
}

// CmdThreadFunction
int32 CmdThread::CmdThreadFunction()
{
	return 0;
}

// SendCommand
status_t CmdThread::SendCommand(CmdThread *thread, Command *cmd)
{
	cmd->sender = this;
	return SendData(thread, cmd->code, &cmd, sizeof(Command *));
}

// ReceiceCommand
Command *CmdThread::ReceiveCommand()
{
	Command *cmd = 0; thread_id thread;
	int32 code = ReceiveData(&thread, &cmd, sizeof(Command *));
	// for compatibility with Thread::Terminate()
	if (!cmd)
		cmd = new Command(code);
	return cmd;
}

// CheckForCommand
Command *CmdThread::CheckForCommand()
{
	if (!HasData())
		return 0;
	return ReceiveCommand();
}

// ReplyCommand
void CmdThread::ReplyCommand(Command *cmd, bool ptr_valid)
{
	if (cmd->sender)
	{
		if (ptr_valid)
			SendCommand(cmd->sender, new CmdReply(cmd));
		else
			SendCommand(cmd->sender, new CmdReply(cmd->code));
	}
}

// CommandProcessed
void CmdThread::CommandProcessed(Command *cmd)
{
	if (cmd->process & PROCESS_REPLY)
		ReplyCommand(cmd, cmd->process & PROCESS_DONT_DELETE);
	if (!(cmd->process & PROCESS_DONT_DELETE))
		delete cmd;
}



