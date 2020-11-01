package tpcb;

import org.apache.commons.cli.*;
import org.postgresql.copy.CopyManager;
import org.postgresql.core.BaseConnection;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import tpcc.data.CopySqlInputStream;

import java.io.IOException;
import java.io.InputStream;
import java.sql.*;
import java.util.*;
import java.util.concurrent.atomic.AtomicLong;

/**
 *
 * https://www.postgresql.org/docs/current/pgbench.html
 *
 * <code>
 *     # 1. 查看内置脚本列表
 *     pgbench4j -b list
 *     # 2. 初始化100倍数据
 *     pgbench4j -i -s 100 -u jdbc:postgresql://ubuntu.wsl:5432/pgbenchdb -U postgres -P postgres
 *     # 3. 两个线程测试一分钟
 *     pgbench4j -s 100 -j 2 -T 60 -u jdbc:postgresql://ubuntu.wsl:5432/pgbenchdb -U postgres -P postgres
 *     # 4. 合并2和3, 初始化100倍数据, 然后启动两个线程, 持续执行测试一分钟
 *     pgbench4j -i -s 100 -j 2 -T 60 -u jdbc:postgresql://ubuntu.wsl:5432/pgbenchdb -U postgres -P postgres
 * </code>
 *
 */
public class PgBench {

    static final Logger log = LoggerFactory.getLogger(PgBench.class);

    static class BuiltinScript {
        String name;			/* very short name for -b ... */
        String desc;			/* short description */
        String script;			/* actual pgbench script */
        String[] sqls;
    }

    enum PartitionMethod {
        NONE,					/* no partitioning */
        RANGE,					/* range partitioning */
        HASH					/* hash partitioning */
        ;
    }

    // d: drop tables , t: truncate tables ,
    // g: generate data on client, G: generate data on server,
    // v: vacuum tables , p: create primary keys , f: create forein keys
    static final String ALL_INIT_STEPS = "dtgGvpf";	/* all possible steps */
    static final String DEFAULT_INIT_STEPS = "dtgvp";	/* default -I setting */

//        #define LOG_STEP_SECONDS	5	/* seconds between log messages */
    static final int DEFAULT_NXACTS	= 10;		/* default nxacts */
//
//        #define MIN_GAUSSIAN_PARAM		2.0 /* minimum parameter for gauss */
//
//        #define MIN_ZIPFIAN_PARAM		1.001	/* minimum parameter for zipfian */
//        #define MAX_ZIPFIAN_PARAM		1000.0	/* maximum parameter for zipfian */

    Options options;

    Map<String, BuiltinScript> scripts = new HashMap<>();
    BuiltinScript tpcbLike;
    BuiltinScript simpleUpdate;
    BuiltinScript selectOnly;

    /* number of transactions per client */
    int			nxacts = 0;
    /* duration in seconds */
    long		duration = 0;
    /* when to stop in micro seconds, under -T */
//    long		end_time = 0;

    /*
     * scaling factor.
     * for example, scale = 10 will make 100_0000 tuples in pgbench_accounts table.
     */
    boolean guestScale = false;
    int			scale = 1;

    /*
     * fillfactor.
     * for example, fillfactor = 90 will use only 90 percent space during inserts and leave 10 percent free.
     */
    int			fillfactor = 100;

    /*
     * use unlogged tables?
     */
    boolean		unlogged_tables = false;

    /*
     * log sampling rate (1.0 = log everything, 0.0 = option not given)
     */
    double		sample_rate = 0.0;

    /*
     * When threads are throttled to a given rate limit,
     * this is the target delay to reach that rate in usec.
     * 0 is the default and means no throttling.
     */
    double		throttle_delay = 0;

    /*
     * Transactions which take longer than this limit (in usec) are counted as late, and reported as such, although they are completed anyway.
     * When throttling is enabled, execution time slots that are more than this late are skipped altogether, and counted separately.
     */
    long		latency_limit = 0;

    /*
     * tablespace selection
     */
    String tablespace = null;
    String index_tablespace = null;

    /*
     * Number of "pgbench_accounts" partitions.
     * 0 is the default and means no partitioning.
     */
    boolean guestPartition;
    int	partitions = 0;

    PartitionMethod partitionMethod = PartitionMethod.NONE;

    /* random seed used to initialize base_random_sequence */
    long		random_seed = -1;

    /*
     * end of configurable parameters
     *********************************************************************/

    int			nclients = 1;		/* number of clients */
    int			nthreads = 1;		/* number of threads */
//    /*
//     * Don't need more threads than there are clients.  (This is not merely an
//     * optimization; throttle_delay is calculated incorrectly below if some
//     * threads have no clients assigned to them.)
//     */
//	if (nthreads > nclients)
//    nthreads = nclients;

    /* Makes little sense to change this.  Change  -s instead */
    int nbranches = 1;
    int ntellers = 10;
    int naccounts = 100000;

    boolean internal_script_used = true;

    boolean initializeMode;
    String initialize_steps = DEFAULT_INIT_STEPS;

//#define nbranches	1			/* Makes little sense to change this.  Change  -s instead */
//        #define ntellers	10
//        #define naccounts	100000
//
//        /*
//         * The scale factor at/beyond which 32bit integers are incapable of storing
//         * 64bit values.
//         *
//         * Although the actual threshold is 21474, we use 20000 because it is easier to
//         * document and remember, and isn't that far away from the real threshold.
//         */
//        #define SCALE_32BIT_THRESHOLD 20000

    String url = "jdbc:postgresql://localhost:5432/postgres";
    String username = "postgres";
    String password = "postgres";

    boolean keepRunning = true;

    public static void main(String[] args) throws Exception {
        PgBench pgBench = new PgBench();
        try {
            pgBench.initBuiltinScripts();
            pgBench.initSupportedOptions();

            if(pgBench.parseCmdLine(args)){
                pgBench.init();
                pgBench.run();
            }
        } catch(Exception e) {
            log.error(e.getMessage(),e);
            pgBench.keepRunning = false;
        }
    }

    private void init() throws SQLException, IOException {
        // 初始化
        log.info("执行初始化工作...");
        long start = System.currentTimeMillis();
        try(Connection conn = getConnection()){
            conn.setAutoCommit(false);
            runInitSteps(conn);
            if (internal_script_used && (guestScale || guestPartition)) {
                getTableInfo(getConnection());
            }
            conn.commit();
        }
        log.info("执行初始化工作完成, 耗时: {} 毫秒", System.currentTimeMillis() - start);
    }

    AtomicLong totalTPS = new AtomicLong();
    AtomicLong transCount = new AtomicLong();
    void run() throws InterruptedException {
        if(nxacts<=0 && duration<=0) return;

        log.info("执行测试... 线程数: {}, 每个线程执行 {} 个事务 或者 持续执行 {} 秒", nthreads, nxacts, duration);
        Thread[] threads = new Thread[nthreads];
        // 创建线程
        for (int i = 0; i < nthreads; i++) {
            Thread thread = new Thread(this::threadRun);
            thread.setName("PGT-bench-" + (i+1) );
            threads[i] = thread;
        }
        // 启动线程
        long start = System.currentTimeMillis();
        for(Thread t : threads){
            t.start();
        }
        // 等待线程结束
        for(Thread t : threads){
            t.join();
        }

        long used = (System.currentTimeMillis() - start) / 1000;
        log.info("执行测试完成, 平均事务数/秒(TPS): {}, 总执行事务数: {}, 总执行时长: {} 秒",
            totalTPS.get(), transCount.get(), used);
    }

    private void threadRun() {
        long threadStart = System.currentTimeMillis();
        log.info("测试线程 {} 运行开始...", Thread.currentThread().getName());
        long trans = 0;
        long lastPrint = 0;
        try (Connection conn = this.getConnection()){
            conn.setAutoCommit(false);

            long start = System.currentTimeMillis();
            while(keepRunning) {
                //int s = num_scripts; // 暂时不支持多个脚本和脚本权重
                // chooseScript // 根据权重来选择SQL脚本
                BuiltinScript script = this.tpcbLike;

                // TODO 这个是不适用预编译语句的, 以后还需要支持预编译语句.
                try(Statement stmt = conn.createStatement()){
                    for(String sql : this.getSqls(script.name)){
                        if(stmt.execute(sql)){
                            consumeResultSet(stmt);
                        }
                    }
                }
                conn.commit();

                trans++;
                long used = (System.currentTimeMillis() - start) / 1000;
                if(used>lastPrint && used % 60 == 0){
                    lastPrint = used;
                    log.info("测试线程 {} 运行进度, 事务数/秒(TPS): {}, 已执行事务数: {}, 已执行时长: {} 秒",
                        Thread.currentThread().getName(), trans/used, trans, used);
                }

                if(nxacts>0 && trans >= nxacts){
                    break;
                }else if(duration>0 && used > duration){
                    break;
                }
            }
        } catch (SQLException e) {
            log.error("测试线程 {} 运行出错: {}", Thread.currentThread().getName(), e.getMessage(), e);
        }finally{
            long used = (System.currentTimeMillis() - threadStart) / 1000;
            transCount.addAndGet(trans);
            totalTPS.addAndGet(used==0 ? 0 : trans/used);
            log.info("测试线程 {} 运行结束: 事务数/秒(TPS): {}, 已执行事务数: {}, 已执行时长: {} 秒",
                Thread.currentThread().getName(), used==0 ? -1 : trans/used, trans, used);
        }
    }

    //
    // ////////////////////////////////////////////////////////////////////////
    //

    static void usage() {
        System.out.printf("%s is a benchmarking tool for PostgreSQL.\n\n"
                + "Usage:\n"
                + "  %s [OPTION]... [DBNAME]\n"
                + "\nInitialization options:\n"
                + "  -i, --initialize         invokes initialization mode\n"
                + "  -I, --init-steps=[" + ALL_INIT_STEPS + "]+ (default \"" + DEFAULT_INIT_STEPS + "\")\n"
                + "                           run selected initialization steps\n"
                + "  -F, --fillfactor=NUM     set fill factor\n"
//            + "  -n, --no-vacuum          do not run VACUUM during initialization\n"
//            + "  -q, --quiet              quiet logging (one message each 5 seconds)\n"
                + "  -s, --scale=NUM          scaling factor\n"
//            + "  --foreign-keys           create foreign key constraints between tables\n"
//                + "  --index-tablespace=TABLESPACE\n"
//                + "                           create indexes in the specified tablespace\n"
                + "  --partition-method=(range|hash)\n"
                + "                           partition pgbench_accounts with this method (default: range)\n"
                + "  --partitions=NUM         partition pgbench_accounts into NUM parts (default: 0)\n"
//                + "  --tablespace=TABLESPACE  create tables in the specified tablespace\n"
//                + "  --unlogged-tables        create tables as unlogged tables\n"
                + "\nOptions to select what to run:\n"
                + "  -b, --builtin=NAME[@W]   add builtin script NAME weighted at W (default: 1)\n"
                + "                           (use \"-b list\" to list available scripts)\n"
//            + "  -f, --file=FILENAME[@W]  add script FILENAME weighted at W (default: 1)\n"
//            + "  -N, --skip-some-updates  skip updates of pgbench_tellers and pgbench_branches\n"
//            + "                           (same as \"-b simple-update\")\n"
//            + "  -S, --select-only        perform SELECT-only transactions\n"
//            + "                           (same as \"-b select-only\")\n"
                + "\nBenchmarking options:\n"
//                + "  -c, --client=NUM         number of concurrent database clients (default: 1)\n"
//            + "  -C, --connect            establish new connection for each transaction\n"
//            + "  -D, --define=VARNAME=VALUE\n"
//            + "                           define variable for use by custom script\n"
            + "  -j, --jobs=NUM           number of threads (default: 1)\n"
//            + "  -l, --log                write transaction times to log file\n"
//            + "  -L, --latency-limit=NUM  count transactions lasting more than NUM ms as late\n"
//            + "  -M, --protocol=simple|extended|prepared\n"
//            + "                           protocol for submitting queries (default: simple)\n"
//            + "  -n, --no-vacuum          do not run VACUUM before tests\n"
//            + "  -P, --progress=NUM       show thread progress report every NUM seconds\n"
//            + "  -r, --report-latencies   report average latency per command\n"
//            + "  -R, --rate=NUM           target rate in transactions per second\n"
//                + "  -s, --scale=NUM          report this scale factor in output\n"
                + "  -t, --transactions=NUM   number of transactions each client runs (default: 10)\n"
                + "  -T, --time=NUM           duration of benchmark test in seconds\n"
//            + "  -v, --vacuum-all         vacuum all four standard tables before tests\n"
//            + "  --aggregate-interval=NUM aggregate data over NUM seconds\n"
//            + "  --log-prefix=PREFIX      prefix for transaction time log file\n"
//            + "                           (default: \"pgbench_log\")\n"
//            + "  --progress-timestamp     use Unix epoch timestamps for progress\n"
//            + "  --random-seed=SEED       set random seed (\"time\", \"rand\", integer)\n"
//            + "  --sampling-rate=NUM      fraction of transactions to log (e.g., 0.01 for 1%%)\n"
//            + "  --show-script=NAME       show builtin script code, then exit\n"
                + "\nCommon options:\n"
//            + "  -d, --debug              print debugging output\n"
//                + "  -h, --host=HOSTNAME      database server host or socket directory\n"
//                + "  -p, --port=PORT          database server port number\n"
                + "  -u, --url=JDBCConnectString  connect as specified url\n"
                + "  -U, --username=USERNAME  connect as specified database user\n"
                + "  -P, --password=PASSWORD  connect as specified database password\n"
//                + "  -V, --version            output version information, then exit\n"
                + "  -?, --help               show this help, then exit\n"
                + "\n"
                + "Report bugs to <%s>.\n"
                + "%s home page: <%s>\n",
            "pgbench4j", "pgbench4j", "support@wapache.org", "pgbench4j", "https://www.wapache.org");
    }

    private Options initSupportedOptions() {
        options = new Options();
        Option help = Option.builder("?")
            .argName("help")
            .hasArg()
            .desc("print this message")
            .optionalArg(true)
            .longOpt("help")
            .build();
        options.addOption(help);

        Option cmd = Option.builder("i")
            .argName("initialize")
            .hasArg()
            .desc("invokes initialization mode")
            .optionalArg(true)
            .longOpt("initialize")
            .numberOfArgs(1)
            .build();
        options.addOption(cmd);

        Option files = Option.builder("I")
            .argName("init-steps")
            .hasArg()
            .desc("")
            .optionalArg(true)
            .longOpt("init-steps")
            .numberOfArgs(1)
            .build();
        options.addOption(files);

        Option outputFile = Option.builder("F")
            .argName("fill factor")
            .hasArg()
            .desc("set fill factor")
            .optionalArg(true)
            .longOpt("fillfactor")
            .numberOfArgs(1)
            .build();
        options.addOption(outputFile);

        Option scale = Option.builder("s")
            .argName("scale")
            .hasArg()
            .desc("scaling factor")
            .optionalArg(true)
            .longOpt("scale")
            .numberOfArgs(1)
            .build();
        options.addOption(scale);

        Option partitionMethod = Option.builder("")
            .argName("partition-method")
            .hasArg()
            .desc("")
            .optionalArg(true)
            .longOpt("partition-method")
            .numberOfArgs(1)
            .build();
        options.addOption(partitionMethod);

        Option partitions = Option.builder("")
            .argName("partitions")
            .hasArg()
            .desc("partition pgbench_accounts into NUM parts (default: 0)")
            .optionalArg(true)
            .longOpt("partitions")
            .numberOfArgs(1)
            .build();
        options.addOption(partitions);

        Option builtin = Option.builder("b")
            .argName("")
            .hasArg()
            .desc("add builtin script NAME weighted at W (default: 1)")
            .optionalArg(true)
            .longOpt("builtin")
            .numberOfArgs(1)
            .build();
        options.addOption(builtin);

        Option jobs = Option.builder("j")
            .argName("jobs")
            .hasArg()
            .desc(" number of threads (default: 1)")
            .optionalArg(true)
            .longOpt("jobs")
            .numberOfArgs(1)
            .build();
        options.addOption(jobs);

        Option transactions = Option.builder("t")
            .argName("")
            .hasArg()
            .desc("number of transactions each client runs (default: 10)")
            .optionalArg(true)
            .longOpt("transactions")
            .numberOfArgs(1)
            .build();
        options.addOption(transactions);

        Option time = Option.builder("T")
            .argName("time")
            .hasArg()
            .desc("duration of benchmark test in seconds")
            .optionalArg(true)
            .longOpt("time")
            .numberOfArgs(1)
            .build();
        options.addOption(time);

        Option url = Option.builder("u")
            .argName("url")
            .hasArg()
            .desc("JDBC connect String")
            .optionalArg(true)
            .longOpt("url")
            .numberOfArgs(1)
            .build();
        options.addOption(url);

        Option username = Option.builder("U")
            .argName("username")
            .hasArg()
            .desc("connect as specified database user")
            .optionalArg(true)
            .longOpt("username")
            .numberOfArgs(1)
            .build();
        options.addOption(username);

        Option password = Option.builder("P")
            .argName("password")
            .hasArg()
            .desc("")
            .optionalArg(true)
            .longOpt("password")
            .numberOfArgs(1)
            .build();
        options.addOption(password);

        return options;
    }

    private boolean parseCmdLine(String[] args) {

        if(args==null || args.length==0){
            usage();
            return false;
        }

        CommandLineParser cmdParser = new DefaultParser();
        CommandLine cmdLine;
        try {
            cmdLine = cmdParser.parse(options, args);
        } catch (ParseException e) {
            log.error("Failed to parse command line", e);
            return false;
        }

        if (cmdLine.hasOption("?")) {
            usage();
            return false;
        }

        if (cmdLine.hasOption("i")) {
            this.initializeMode = true;
        }

        if (cmdLine.hasOption("I")) {
            this.initialize_steps = cmdLine.getOptionValue("I", "");
        }
        if (cmdLine.hasOption("F")) {
            this.fillfactor = Integer.parseInt(cmdLine.getOptionValue("F", "100"));
        }
        if (cmdLine.hasOption("s")) {
            this.scale = Integer.parseInt(cmdLine.getOptionValue("s", "1"));
            if(this.scale<0){
                this.guestScale = true;
                this.scale = 1;
            }
        }
        if (cmdLine.hasOption("partition-method")) {
            this.partitionMethod = PartitionMethod.valueOf(cmdLine.getOptionValue("partition-method"));
        }
        if (cmdLine.hasOption("partitions")) {
            this.partitions = Integer.parseInt(cmdLine.getOptionValue("partitions", "1"));
            if(this.partitions<0){
                this.guestPartition = true;
                this.partitions = 0;
            }
        }

        if (cmdLine.hasOption("b")) {
            String[] scriptNames = cmdLine.getOptionValues("b");
            for(String name : scriptNames){
                if("list".contains(name)){
                    this.scripts.keySet().forEach(System.out::println);
                    return false;
                }
                // TODO 暂不支持指定SQL脚本
            }
        }

        if (cmdLine.hasOption("j")) {
            this.nthreads = Integer.parseInt(cmdLine.getOptionValue("j", "1"));
        }
        if (cmdLine.hasOption("t")) {
            this.nxacts = Integer.parseInt(cmdLine.getOptionValue("t", "1000")); // 默认10000事务
            this.duration = 0;
        }
        if (cmdLine.hasOption("T")) {
            String tmp = cmdLine.getOptionValue("T", "720"); // 默认12分钟
            this.duration = Long.parseLong(tmp);
            this.nxacts = 0;
        }

        if (cmdLine.hasOption("u")) {
            this.url = cmdLine.getOptionValue("u");
        }

        if (cmdLine.hasOption("U")) {
            this.username = cmdLine.getOptionValue("U");
        }

        if (cmdLine.hasOption("P")) {
            this.password = cmdLine.getOptionValue("P");
        }

        if(!initializeMode && nxacts==0 && duration == 0){
            log.error("非初始化模式下, 事务数 和 执行时长 不能同时为0: 事务数: {}, 执行时长: {} 秒",
                nxacts, duration);
            return false;
        }

        return true;
    }
    
    //
    // ////////////////////////////////////////////////////////////////////////
    //

    void runInitSteps(Connection conn) throws SQLException, IOException {

        if(!initializeMode) return;

//        weight = parseScriptWeight(optarg, &script);
//        process_builtin(findBuiltin(script), weight);

        checkInitSteps();
        for(char c : initialize_steps.toCharArray()){
            String op = null;
            long start = System.currentTimeMillis();
            switch (c)
            {
                case 'd':
                    op = "drop tables";
                    initDropTables(conn);
                    break;
                case 't':
                    op = "create tables";
                    initCreateTables(conn);
                    break;
                case 'g':
                    op = "client-side generate";
                    initGenerateDataClientSide(conn);
                    break;
                case 'G':
                    op = "server-side generate";
                    initGenerateDataServerSide(conn);
                    break;
                case 'v':
                    op = "vacuum";
                    conn.commit();
                    initVacuum();
                    break;
                case 'p':
                    op = "primary keys";
                    initCreatePKeys(conn);
                    break;
                case 'f':
                    op = "foreign keys";
                    initCreateFKeys(conn);
                    break;
                case ' ':
                    break;			/* ignore */
                default:
                    log.error("unrecognized initialization step \"{}\"", c);
                    System.exit(1);
            }
            log.info("执行 {} 操作耗时 {} 毫秒", op, System.currentTimeMillis() - start);
        }
    }

    void initBuiltinScripts() {

        // pgbench会解析这些sql模板, 替换里边的变量, 然后再执行
        // 要完全支持这个机制, 工作量有点大, 直接写java拼接吧....

        tpcbLike = new BuiltinScript();
        tpcbLike.name = "tpcb-like";
        tpcbLike.desc = "<builtin: TPC-B (sort of)>";
        tpcbLike.sqls = new String[]{
            "UPDATE pgbench_accounts SET abalance = abalance + :delta WHERE aid = :aid",
            "SELECT abalance FROM pgbench_accounts WHERE aid = :aid",
            "UPDATE pgbench_tellers SET tbalance = tbalance + :delta WHERE tid = :tid",
            "UPDATE pgbench_branches SET bbalance = bbalance + :delta WHERE bid = :bid",
            "INSERT INTO pgbench_history (tid, bid, aid, delta, mtime) VALUES (:tid, :bid, :aid, :delta, CURRENT_TIMESTAMP)"
        };
        tpcbLike.script =
            "\\set aid random(1, " + naccounts + " * :scale)\n"+
                "\\set bid random(1, " + nbranches + " * :scale)\n"+
                "\\set tid random(1, " + ntellers + " * :scale)\n"+
                "\\set delta random(-5000, 5000)\n"+
                "BEGIN;\n"+
                tpcbLike.sqls[0]+";\n"+
                tpcbLike.sqls[1]+";\n"+
                tpcbLike.sqls[2]+";\n"+
                tpcbLike.sqls[3]+";\n"+
                tpcbLike.sqls[4]+";\n"+
                "END;\n"
        ;
        scripts.put(tpcbLike.name, tpcbLike);


        simpleUpdate = new BuiltinScript();
        simpleUpdate.name = "simple-update";
        simpleUpdate.desc = "<builtin: simple update>";
        simpleUpdate.sqls = new String[]{
            "UPDATE pgbench_accounts SET abalance = abalance + :delta WHERE aid = :aid",
            "SELECT abalance FROM pgbench_accounts WHERE aid = :aid",
            "INSERT INTO pgbench_history (tid, bid, aid, delta, mtime) VALUES (:tid, :bid, :aid, :delta, CURRENT_TIMESTAMP)"
        };
        simpleUpdate.script =
            "\\set aid random(1, " + naccounts + " * :scale)\n"+
                "\\set bid random(1, " + nbranches + " * :scale)\n"+
                "\\set tid random(1, " + ntellers + " * :scale)\n"+
                "\\set delta random(-5000, 5000)\n"+
                "BEGIN;\n"+
                simpleUpdate.sqls[0]+";\n"+
                simpleUpdate.sqls[1]+";\n"+
                simpleUpdate.sqls[2]+";\n"+
                "END;\n"
        ;
        scripts.put(simpleUpdate.name, simpleUpdate);


        selectOnly = new BuiltinScript();
        selectOnly.name = "select-only";
        selectOnly.desc = "<builtin: select only>";
        selectOnly.sqls = new String[]{
            "SELECT abalance FROM pgbench_accounts WHERE aid = :aid"
        };
        selectOnly.script =
            "\\set aid random(1, " + naccounts + " * :scale)\n"+
                "BEGIN;\n"+
                selectOnly.sqls[0]+";\n"+
                "END;\n"
        ;
        scripts.put(selectOnly.name, selectOnly);

    }

    /**
     * Validate an initialization-steps string
     *
     * (We could just leave it to runInitSteps() to fail if there are wrong
     * characters, but since initialization can take awhile, it seems friendlier
     * to check during option parsing.)
     */
    void checkInitSteps() {
        if(initialize_steps==null || initialize_steps.isEmpty()){
            log.error("no initialization steps specified");
            System.exit(1);
        }
        for(char c : initialize_steps.toCharArray()){
            if(ALL_INIT_STEPS.indexOf(c)==-1){
                log.error("unrecognized initialization step: {}, Allowed step characters are: {}", c, ALL_INIT_STEPS);
                System.exit(1);
            }
        }
    }

    void initDropTables(Connection conn) throws SQLException {
        executeStatement(conn, "drop table if exists "
            + "pgbench_accounts, "
            + "pgbench_branches, "
            + "pgbench_history, "
            + "pgbench_tellers");
    }

    /**
     * Truncate away any old data, in one command in case there are foreign keys
     */
    void initTruncateTables(Connection conn)
    {
        tryExecuteStatement(conn, "truncate table "
            + "pgbench_accounts, "
            + "pgbench_branches, "
            + "pgbench_history, "
            + "pgbench_tellers");
    }

    void initCreateTables(Connection conn) throws SQLException {
        for (DdlInfo info : DdlInfo.tables) {

            String sql = String.format("create%s table %s (%s)",
                unlogged_tables ? " unlogged" : "",
                info.table,
                info.columns
            );

            if (partitionMethod != PartitionMethod.NONE && "pgbench_accounts".equals(info.table)){
                sql += String.format(" partition by %s (aid)", partitionMethod.name().toLowerCase());
            } else if (info.fillFactor==1){
                sql += String.format(" with (fillfactor=%d)", fillfactor);
            }

            if (tablespace != null){
                sql += String.format(" tablespace %s", tablespace);
            }

            executeStatement(conn, sql);
        }

        if (partitionMethod != PartitionMethod.NONE){
            createPartitions(conn);
        }
    }

    void createPartitions(Connection conn) throws SQLException {
        for (int p = 1; p <= partitions; p++) {
            switch(partitionMethod) {
                case RANGE: {

                    long part_size = (naccounts * scale + partitions - 1) / partitions;
                    String sql = String.format("create%s table pgbench_accounts_%d\n" +
                            "  partition of pgbench_accounts\n" +
                            "  for values from (%s) to (%s)" +
                            " with (fillfactor=%d)",
                        unlogged_tables ? " unlogged" : "",
                        p,
                        p == 1 ? "minvalue" : String.valueOf((p - 1) * part_size + 1),
                        p < partitions ? String.valueOf(p * part_size + 1) : "maxvalue",
                        fillfactor
                    );
                    executeStatement(conn, sql);
                }
                break;
                case HASH: {
                    String sql = String.format(
                        "create%s table pgbench_accounts_%d\n" +
                            "  partition of pgbench_accounts\n" +
                            "  for values with (modulus %d, remainder %d)" +
                            " with (fillfactor=%d)",
                        unlogged_tables ? " unlogged" : "",
                        p,
                        partitions,
                        p - 1,
                        fillfactor
                    );
                    executeStatement(conn, sql);
                }
                break;
                case NONE:
                    break;
            }
        }
    }

    /**
     * Fill the standard tables with some data generated and sent from the client
     */
    void initGenerateDataClientSide(Connection conn) throws SQLException, IOException {

        this.initTruncateTables(conn);

        // fill branches, tellers, accounts in that order in case foreign keys already exist
        for (int i = 1; i <= nbranches * scale; i++) {
            /* "filler" column defaults to NULL */
            executeStatement(conn, String.format("insert into pgbench_branches(bid,bbalance) values(%d,0)", i));
        }

        for (int i = 0; i < ntellers * scale; i++) {
//            /* "filler" column defaults to NULL */
            executeStatement(conn, String.format("insert into pgbench_tellers(tid,bid,tbalance) values (%d,%d,0)", i + 1, i / ntellers + 1));
        }

        // accounts is big enough to be worth using COPY and tracking runtime
        long count = naccounts * scale;
        long start = System.currentTimeMillis();
        CopySqlInputStream inputStream = new CopySqlInputStream();
        Thread thread = new Thread(()->{
            try{
                for (long k = 0; k < count; k++) {
                    long j = k + 1;

                    if(!keepRunning) {
                        inputStream.pushEnd();
                        if(conn!=null && !conn.isClosed()) conn.rollback();
                        break;
                    }

                    // "filler" column defaults to blank padded empty string
                    inputStream.pushLine(String.format( "%d\t%d\t%d\t\n", j, k / naccounts + 1, 0));

                    if(j>=count){
                        inputStream.pushEnd();
                    }
                    if(j % 100_0000 == 0){
                        long used = (System.currentTimeMillis() - start) / 1000;
                        log.info(
                            "生成accounts数据进度: 已生成 {} 万条数据, 累计耗时: {} 秒, 生成速度: {} 条/秒",
                            j/10000,
                            used,
                            used==0 ? 0 : j/used
                        );
                    }
                }
            } catch (InterruptedException | SQLException e) {
                throw new RuntimeException(e);
            }
        });
        thread.setName("PGT-copy");
        thread.setDaemon(true);
        thread.start();

        copyIn(conn, "pgbench_accounts", "(aid,bid,abalance,filler)", inputStream);

        long used = (System.currentTimeMillis() - start) / 1000;
        log.info(
            "生成accounts数据{}: 已生成 {} 万条数据, 累计耗时: {} 秒, 写入速度: {} 条/秒",
            keepRunning ? "完成":"中断",
            count/10000,
            used,
            used==0 ? -1 : count/used
        );
    }

    void initGenerateDataServerSide(Connection conn) throws SQLException {

        String sql1 = String.format("insert into pgbench_branches(bid,bbalance) " +
            "select bid, 0 "+
            "from generate_series(1, %d) as bid", nbranches * scale);

        executeStatement(conn, sql1);

        String sql2 = String.format(
            "insert into pgbench_branches(bid,bbalance) " +
                "select bid, 0 " +
                "from generate_series(1, %d) as bid", nbranches * scale);

        executeStatement(conn, sql2);

        String sql3 = String.format(
            "insert into pgbench_tellers(tid,bid,tbalance) " +
                "select tid, (tid - 1) / %d + 1, 0 " +
                "from generate_series(1, %d) as tid", ntellers, ntellers * scale);

        executeStatement(conn, sql3);

        String sql4 = String.format(
            "insert into pgbench_accounts(aid,bid,abalance,filler) " +
                "select aid, (aid - 1) / %d + 1, 0, '' " +
                "from generate_series(1, %d) as aid",
            naccounts, naccounts * scale
        );

        executeStatement(conn, sql4);

    }

    void initVacuum() throws SQLException {
        // VACUUM cannot run inside a transaction block
        try(Connection conn = getConnection()) {
            executeStatements(conn,
                "vacuum analyze pgbench_branches",
                "vacuum analyze pgbench_tellers",
                "vacuum analyze pgbench_accounts",
                "vacuum analyze pgbench_history"
            );
        }
    }

    /**
     * Create primary keys on the standard tables
     */
    void initCreatePKeys(Connection conn) throws SQLException {
        String DDLINDEXes[] = {
            "alter table pgbench_branches add primary key (bid)",
            "alter table pgbench_tellers add primary key (tid)",
            "alter table pgbench_accounts add primary key (aid)"
        };

        for (String index : DDLINDEXes) {
            String sql = index + (index_tablespace==null?"": String.format(" using index tablespace %s", index_tablespace));
            executeStatement(conn, sql);
        }
    }

    void initCreateFKeys(Connection conn) throws SQLException {

        String DDLKEYs[] = {
            "alter table pgbench_tellers add constraint pgbench_tellers_bid_fkey foreign key (bid) references pgbench_branches",
            "alter table pgbench_accounts add constraint pgbench_accounts_bid_fkey foreign key (bid) references pgbench_branches",
            "alter table pgbench_history add constraint pgbench_history_bid_fkey foreign key (bid) references pgbench_branches",
            "alter table pgbench_history add constraint pgbench_history_tid_fkey foreign key (tid) references pgbench_tellers",
            "alter table pgbench_history add constraint pgbench_history_aid_fkey foreign key (aid) references pgbench_accounts"
        };

        executeStatements(conn, DDLKEYs);

    }

//    static ParsedScript sql_script[MAX_SCRIPTS];	/* SQL script files */
//    static int	num_scripts;		/* number of scripts in sql_script[] */
//    static int64 total_weight = 0;

//    /* return a script number with a weighted choice. */
//    static int
//    chooseScript(TState *thread)
//    {
//        int			i = 0;
//        int64		w;
//
//        if (num_scripts == 1)
//            return 0;
//
//        w = getrand(&thread->ts_choose_rs, 0, total_weight - 1);
//        do
//        {
//            w -= sql_script[i++].weight;
//        } while (w >= 0);
//
//        return i - 1;
//    }

    String[] getSqls(String name){
        switch (name){
            case "tpcb-like"    : return getTpcbLikeSqls();
            case "simple-update": return getSimpleUpdateSqls();
            case "select-only"  : return getSelectOnlySqls();
            default:
                return new String[]{};
        }
    }

    String[] getTpcbLikeSqls(){

        Random random = new Random();

        long aid = 1 + random.nextInt(naccounts * scale);
        long bid = 1 + random.nextInt(nbranches * scale);
        long tid = 1 + random.nextInt(ntellers * scale);
        int delta = 5000 - (1 + random.nextInt(10000));

        String[] sqls = new String[tpcbLike.sqls.length];
        for (int i = 0; i < sqls.length; i++) {
            sqls[i] = tpcbLike.sqls[i]
                .replace(":aid", String.valueOf(aid))
                .replace(":bid", String.valueOf(bid))
                .replace(":tid", String.valueOf(tid))
                .replace(":delta", String.valueOf(delta))
            ;
        }

        return sqls;
    }

    String[] getSimpleUpdateSqls(){

        Random random = new Random();

        long aid = 1 + random.nextInt(naccounts * scale);
        long bid = 1 + random.nextInt(nbranches * scale);
        long tid = 1 + random.nextInt(ntellers * scale);
        int delta = 5000 - (1 + random.nextInt(10000));

        String[] sqls = new String[simpleUpdate.sqls.length];
        for (int i = 0; i < sqls.length; i++) {
            sqls[i] = simpleUpdate.sqls[i]
                .replace(":aid", String.valueOf(aid))
                .replace(":bid", String.valueOf(bid))
                .replace(":tid", String.valueOf(tid))
                .replace(":delta", String.valueOf(delta))
            ;
        }

        return sqls;
    }

    String[] getSelectOnlySqls(){

        Random random = new Random();

        long aid = 1 + random.nextInt(naccounts * scale);

        String[] sqls = new String[selectOnly.sqls.length];
        for (int i = 0; i < sqls.length; i++) {
            sqls[i] = selectOnly.sqls[i]
                .replace(":aid", String.valueOf(aid))
            ;
        }

        return sqls;
    }

    void getTableInfo(Connection conn){

        String sql = "select count(*) from pgbench_branches";
        // TODO 这个值就是scale的值.
        try( Statement stmt = conn.createStatement(); ResultSet rs = stmt.executeQuery(sql); ){
            if(rs.next()){
                scale = rs.getInt(1);
            }else{

//            pg_log_fatal("could not count number of branches: %s", PQerrorMessage(con));
//
//            if (sqlState && strcmp(sqlState, ERRCODE_UNDEFINED_TABLE) == 0)
//                pg_log_info("Perhaps you need to do initialization (\"pgbench -i\") in database \"%s\"",
//                    PQdb(con));
                log.error("表pgbench_branches存在但是没有数据, 可能需要执行 (\"pgbench -i\") 初始化数据。");
                System.exit(1);
            }
        } catch (SQLException throwables) {
            throwables.printStackTrace();
            System.exit(1);
        }

        /*
         * Get the partition information for the first "pgbench_accounts" table
         * found in search_path.
         *
         * The result is empty if no "pgbench_accounts" is found.
         *
         * Otherwise, it always returns one row even if the table is not
         * partitioned (in which case the partition strategy is NULL).
         *
         * The number of partitions can be 0 even for partitioned tables, if no
         * partition is attached.
         *
         * We assume no partitioning on any failure, so as to avoid failing on an
         * old version without "pg_partitioned_table".
         */
        String sql2 =
            "select o.n, p.partstrat, pg_catalog.count(i.inhparent) " +
            "from pg_catalog.pg_class as c " +
            "join pg_catalog.pg_namespace as n on (n.oid = c.relnamespace) " +
            "cross join lateral (select pg_catalog.array_position(pg_catalog.current_schemas(true), n.nspname)) as o(n) " +
            "left join pg_catalog.pg_partitioned_table as p on (p.partrelid = c.oid) " +
            "left join pg_catalog.pg_inherits as i on (c.oid = i.inhparent) " +
            "where c.relname = 'pgbench_accounts' and o.n is not null " +
            "group by 1, 2 " +
            "order by 1 asc " +
            "limit 1";

        try( Statement stmt = conn.createStatement(); ResultSet rs = stmt.executeQuery(sql2); ){
            if(rs.next()){
                String value = rs.getString(1);
                if(value==null){
                    partitions = 0;
                    partitionMethod = PartitionMethod.NONE;
                }else if("r".equalsIgnoreCase(value)){
                    partitions = rs.getInt(2);
                    partitionMethod = PartitionMethod.RANGE;
                }else if("h".equalsIgnoreCase(value)){
                    partitions = rs.getInt(2);
                    partitionMethod = PartitionMethod.HASH;
                }else{
                    log.error("表pgbench_accounts的分区方法暂不支持: {}", value);
//                    /* possibly a newer version with new partition method */
//                    pg_log_fatal("unexpected partition method: \"%s\"", ps);
                    System.exit(1);
                }
            }else{
                log.error("表pgbench_accounts不存在, 可能需要执行 (\"pgbench -i\") 初始化数据。");
//            pg_log_fatal("no pgbench_accounts table found in search_path");
//            pg_log_info("Perhaps you need to do initialization (\"pgbench -i\") in database \"%s\".", PQdb(con));
                System.exit(1);
            }
        } catch (SQLException throwables) {
            throwables.printStackTrace();

            partitions = 0;
            partitionMethod = PartitionMethod.NONE;

        }

    }

    Connection getConnection() throws SQLException {
        return DriverManager.getConnection(url, username, password);
    }

    void executeStatements(Connection conn, String... sqls) throws SQLException {
        try(Statement stmt = conn.createStatement()){
            for( String sql : sqls ){
                log.debug("执行SQL: {}", sql);
                stmt.execute(sql);
            }
        }
    }

    void executeStatement(Connection conn, String sql) throws SQLException {
        log.debug("执行SQL: {}", sql);
        try(Statement stmt = conn.createStatement()){
            stmt.execute(sql);
        }
    }

    void tryExecuteStatement(Connection conn, String sql){
        try(Statement stmt = conn.createStatement()){
            log.debug("执行SQL: {}", sql);
            stmt.execute(sql);
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    void copyIn(Connection conn, String tableName, String fields, InputStream inputStream) throws SQLException, IOException {
        CopyManager copyManager = new CopyManager((BaseConnection)conn);
        copyManager.copyIn("COPY " + tableName + " "+ (fields==null?"":fields) + " FROM STDIN", inputStream);
    }

    void consumeResultSet(Statement stmt) throws SQLException {
        try(ResultSet rs = stmt.getResultSet()){
            consumeResultSet(rs);
        }
    }

    void consumeResultSet(ResultSet rs) throws SQLException {
        if(rs!=null){
            ResultSetMetaData meta = rs.getMetaData();
            while(rs.next()){
                for(int i=1;i<=meta.getColumnCount();i++){
                    rs.getObject(i);
                }
            }
        }
    }

    static class DdlInfo {

        String table;
        String columns;
        int fillFactor;

        static List<DdlInfo> tables = new ArrayList<>();
        static {

            DdlInfo history = new DdlInfo();
            history.table = "pgbench_history";
            history.columns = "tid int,bid int,aid bigint,delta int,mtime timestamp,filler char(22)";
            history.fillFactor = 0;
            tables.add(history);

            DdlInfo tellers = new DdlInfo();
            tellers.table = "pgbench_tellers";
            tellers.columns = "tid int not null,bid int,tbalance int,filler char(84)";
            tellers.fillFactor = 1;
            tables.add(tellers);

            DdlInfo accounts = new DdlInfo();
            accounts.table = "pgbench_accounts";
            accounts.columns = "aid bigint not null,bid int,abalance int,filler char(84)";
            accounts.fillFactor = 1;
            tables.add(accounts);

            DdlInfo branches = new DdlInfo();
            branches.table = "pgbench_branches";
            branches.columns = "bid int not null,bbalance int,filler char(88)";
            branches.fillFactor = 1;
            tables.add(branches);

        }

    }

}

/*

JDBC设置变量

"a series of statements like" -

you can send all those queries in a single roundtrip if you delimit them with ;.
Although the JDBC specification does not allow running multiple statements with a single execute() call, the Postgres JDBC driver does support that,
e.g. stmt.execute("set timezone='...'; set work_mem='64MB'; set ...;")

 */