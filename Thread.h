// Thread.h

#ifndef THREAD_H
#define THREAD_H

#include <kernel/OS.h>
#include <interface/Point.h>

// command codes for send/receive
enum
{
	CMD_TERMINATE			= -2,
	CMD_REPLY				= -1,
	CMD_NONE				= 0,
	CMD_DRAW_FUNCTION		= 1,
	CMD_PRE_CALCULATE		= 2,
	CMD_POST_CALCULATE		= 3
};

// Thread - a simple thread wrapper class
class Thread
{
public:
	Thread(const char *name, int32 priority);
	virtual ~Thread();
	status_t Resume() { return resume_thread(id); };
	status_t Suspend() { return suspend_thread(id); };
	status_t WaitFor(status_t *exit_value)
		{ return wait_for_thread(id, exit_value); };
	void Exit(status_t ret_val);
	status_t Kill();
	void Die();
	status_t SendData(Thread *thread, int32 code, void *buffer, size_t size)
		{ return send_data(thread->id, code, buffer, size); };
	int32 ReceiveData(thread_id *sender, void *buffer, size_t size)
		{ return receive_data(sender, buffer, size); };
	bool HasData()
		{ return has_data(id); };
	status_t Terminate()
		{ return SendData(this, CMD_TERMINATE, 0, 0); };
	virtual int32 ThreadFunction() = 0;
private:
	static int32 run_thread(void *data);
protected:
	bool		go;
private:
	thread_id	id;
};

struct Command;

// CmdThread - a thread class communicating with Command derived struct's
class CmdThread : public Thread
{
public:
	CmdThread(const char *name, int32 priority) : Thread(name, priority) {};
	virtual int32 ThreadFunction();
	virtual int32 CmdThreadFunction();
	status_t SendCommand(CmdThread *thread, Command *cmd);
	Command *ReceiveCommand();
	Command *CheckForCommand();
	void ReplyCommand(Command *cmd, bool ptr_valid = true);
	void CommandProcessed(Command *cmd);
};

// Command flags
enum
{
	PROCESS_STANDARD	= 0x0,
	PROCESS_DONT_DELETE = 0x1,
	PROCESS_REPLY		= 0x2
};

// Command
struct Command
{
	Command(int32 ccode, Command *creply = 0, int32 cprocess = PROCESS_STANDARD,
		int32 creply_code = CMD_NONE)
	{
		code = ccode;
		reply = creply;
		reply_code = creply_code;
		sender = 0;
		process = cprocess;
	};
	virtual ~Command() {};

	int32		code;
	Command		*reply;
	int			reply_code;
	CmdThread	*sender;
	int32		process;
};

// CmdTerminate
struct CmdTerminate : Command
{
	CmdTerminate() : Command(CMD_TERMINATE) {};
};

// CmdReply
struct CmdReply : Command
{
	CmdReply(Command *cmd) : Command(CMD_REPLY, cmd, PROCESS_STANDARD,
									cmd ? cmd->code : CMD_NONE) {};
	CmdReply(int32 code) : Command(CMD_REPLY, 0, PROCESS_STANDARD, code) {};
};

// CmdDrawFunction
struct CmdDrawFunction : public Command
{
	CmdDrawFunction() : Command(CMD_DRAW_FUNCTION, 0, PROCESS_REPLY) {};
};

// CmdPostCalculate
struct CmdPostCalculate : Command
{
	CmdPostCalculate(int32 ccount)
		: Command(CMD_POST_CALCULATE, 0, PROCESS_DONT_DELETE | PROCESS_REPLY)
		{ count = ccount; results = new float[count]; };
	virtual ~CmdPostCalculate() { delete[] results; };
	BPoint	*points;
	float	*results;
	int32	count;
};


#endif	// THREAD_H

