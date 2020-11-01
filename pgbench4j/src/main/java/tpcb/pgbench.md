常见的开源数据库的基准测试工具有benchmarksql、sysbench等，PostgreSQL自带运行基准测试的简单程序pgbench。
pgbench是一个类TPC-B的基准测试工具，可以执行内置的测试脚本，也可以自定义脚本文件。

使用pgbench进行测试

初始化测试数据

pgbench 的内嵌脚本需要 4 张表： pgbench_branches 、 pgbench_tellers 、 pgbench_accounts
和 pgbench_history。 使用 pgbench 初始化测试数据， pgbench 会自动去创建这些表并生成测试数据。

```shell script
pgbench -i -s 2 -F 80 -U dev -d devdb
```

选择 3 种内置脚本混合进行测试，并在脚本名称后面加上＠符号， ＠符号后面加一个脚本运行比例的权重 的整数值

```shell script

pgbench -b simple-update@2 -b select-only@8 -b tpcb@0 -U dev devdb

```

[如何正确地进行pgbench](https://www.jianshu.com/p/b0c0ef9d2f66)

状态机:

```

# Select transaction (script) to run.
CSTATE_CHOOSE_SCRIPT
	# If time is over, we're done; otherwise, get ready to start a new transaction, or to get throttled if that's requested.
	timer_exceeded 
	? CSTATE_FINISHED 
	: throttle_delay > 0 ? CSTATE_PREPARE_THROTTLE : CSTATE_START_TX;

# Start new transaction (script)
CSTATE_START_TX
	# 连接失败
	CSTATE_ABORTED
	# Begin with the first command
	CSTATE_START_COMMAND
	
# Handle throttling once per transaction by sleeping.
CSTATE_PREPARE_THROTTLE
	CSTATE_FINISHED
	CSTATE_THROTTLE
	while (thread->throttle_trigger < now_us - latency_limit && (nxacts <= 0 || st->cnt < nxacts))
	{
		processXactStats(thread, st, &now, true, agg);
		...
	}

# Wait until it's time to start next transaction.
CSTATE_THROTTLE
	timer_exceeded 
	? CSTATE_FINISHED 
	: CSTATE_START_TX;

# Send a command to server (or execute a meta-command)
CSTATE_START_COMMAND
	# 没有需要执行的command了
	CSTATE_END_TX
	# 执行command失败
	CSTATE_ABORTED
	# 执行成功
	CSTATE_WAIT_RESULT
	# 如果command是sleep
	CSTATE_SLEEP
	# 
	CSTATE_END_COMMAND

# non executed conditional branch, 如果脚本里有if-else等meta command的话才用得到
CSTATE_SKIP_COMMAND

# Wait for the current SQL command to complete
CSTATE_WAIT_RESULT
	CSTATE_ABORTED
	# 成功读取执行结果
	CSTATE_END_COMMAND

CSTATE_SLEEP
	CSTATE_END_COMMAND
	
# End of command: record stats and proceed to next command.
CSTATE_END_COMMAND
	# 开始执行下一条command
	CSTATE_START_COMMAND
	CSTATE_SKIP_COMMAND

# End of transaction (end of script, really).
CSTATE_END_TX
	CSTATE_FINISHED
	CSTATE_CHOOSE_SCRIPT

CSTATE_ABORTED
CSTATE_FINISHED


```