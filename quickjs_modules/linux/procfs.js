import * as std from 'std';



    // proc[P_CPUINFO].filename = "/proc/cpuinfo";
    // proc[P_STAT].filename = "/proc/stat";
    // proc[P_VERSION].filename = "/proc/version";
    // proc[P_MEMINFO].filename = "/proc/meminfo";
    // proc[P_UPTIME].filename = "/proc/uptime";
    // proc[P_LOADAVG].filename = "/proc/loadavg";
    // proc[P_NFS].filename = "/proc/net/rpc/nfs";
    // proc[P_NFSD].filename = "/proc/net/rpc/nfsd";
    // proc[P_VMSTAT].filename = "/proc/vmstat";

let  CPUINFO	= 'cpuinfo';
let  STAT		= 'stat'; // '/proc/stat';
let  VERSION	= 'version'; // '/proc/version';
let  MEMINFO    = 'meminfo'; // '/proc/meminfo';
let  UPTIME     = 'uptime'; // '/proc/uptime';
let  LOADAVG    = 'loadavg'; // '/proc/loadavg';
let  NFS        = 'nfs'; // '/proc/net/rpc/nfs';
let  NFSD       = 'nfsd'; // '/proc/net/rpc/nfsd';
let  VMSTAT     = 'vmstat'; // '/proc/vmstat';	/* new in 13h */

var procs = {};

// FILE *fp;
// char *filename;
// int size;
// int lines;
// char *line[PROC_MAXLINES];
// char *buf;
// int read_this_interval;	/* track updates for each update to stop  double data collection */

function get_cpuinfo() {

    var cpuinfo = procs[CPUINFO] = {
        filename: "/proc/cpuinfo",
        processors: []
    };
    
    
    var line, 
        re  = /\s*:\s*/,
        processor, 
        file = std.popen("sudo cat "+cpuinfo.filename, "r")
    ;
    while ((line = file.getline()) != null) {
        // console.log('line', line);
        if(line.length==0){
            continue;
        }
        if(line.indexOf('processor')==0){
            processor = {};
            cpuinfo.processors.push(processor);
        }
        let part = line.split(re);
        processor[part[0]] = part[1];
    }
    file.close();
    
    // 对收集到的processors整理,归类,统计
        

    // cpu_snap
    // snap_average() = 每个cpu的user+kernal的和的平均值

}

// struct {
//     double user;
//     double kernel;
//     double iowait;
//     double idle;
//     double steal;
// } cpu_snap[MAX_SNAPS];

// struct procsinfo {
//     int pi_pid;
//     char pi_comm[64];
//     char pi_state;
//     int pi_ppid;
//     int pi_pgrp;
//     int pi_session;
//     int pi_tty_nr;
//     int pi_tty_pgrp;
//     unsigned long pi_flags;
//     unsigned long pi_minflt;
//     unsigned long pi_cmin_flt;
//     unsigned long pi_majflt;
//     unsigned long pi_cmaj_flt;
//     unsigned long pi_utime;
//     unsigned long pi_stime;
//     long pi_cutime;
//     long pi_cstime;
//     long pi_pri;
//     long pi_nice;
// #ifdef PRE_KERNEL_2_6_18
//     long junk /* removed */ ;
// #else
//     long pi_num_threads;
// #endif
//     long pi_it_real_value;
//     unsigned long pi_start_time;
//     unsigned long pi_vsize;
//     long pi_rss;		/* - 3 */
//     unsigned long pi_rlim_cur;
//     unsigned long pi_start_code;
//     unsigned long pi_end_code;
//     unsigned long pi_start_stack;
//     unsigned long pi_esp;
//     unsigned long pi_eip;
//     /* The signal information here is obsolete. */
//     unsigned long pi_pending_signal;
//     unsigned long pi_blocked_sig;
//     unsigned long pi_sigign;
//     unsigned long pi_sigcatch;
//     unsigned long pi_wchan;
//     unsigned long pi_nswap;
//     unsigned long pi_cnswap;
//     int pi_exit_signal;
//     int pi_cpu;
// #ifndef PRE_KERNEL_2_6_18
//     unsigned long pi_rt_priority;
//     unsigned long pi_policy;
//     unsigned long long pi_delayacct_blkio_ticks;
// #endif
//     unsigned long statm_size;	/* total program size */
//     unsigned long statm_resident;	/* resident set size */
//     unsigned long statm_share;	/* shared pages */
//     unsigned long statm_trs;	/* text (code) */
//     unsigned long statm_drs;	/* data/stack */
//     unsigned long statm_lrs;	/* library */
//     unsigned long statm_dt;	/* dirty pages */

//     unsigned long long read_io;	/* storage read bytes */
//     unsigned long long write_io;	/* storage write bytes */
// };

// /* Main data structure for collected stats.
//  * Two versions are previous and current data.
//  * Often its the difference that is printed.
//  * The pointers are swaped i.e. current becomes the previous
//  * and the previous over written rather than moving data around.
//  */
// struct cpu_stat {		/* changed the order here to match this years kernel (man 5 /proc/stat) */
//     long long user;
//     long long nice;
//     long long sys;
//     long long idle;
//     long long wait;		/* for IO */
//     long long irq;
//     long long softirq;
//     long long steal;
//     long long guest;
//     long long guest_nice;
//     /* below are non-cpu based numbers in the same file */
//     long long intr;
//     long long ctxt;
//     long long procs;
//     long long running;
//     long long blocked;
//     float uptime;
//     float idletime;
//     float mins1;
//     float mins5;
//     float mins15;
// };

// #define ulong unsigned long
// struct dsk_stat {
//     char dk_name[32];
//     int dk_major;
//     int dk_minor;
//     long dk_noinfo;
//     ulong dk_reads;
//     ulong dk_rmerge;
//     ulong dk_rmsec;
//     ulong dk_rkb;
//     ulong dk_writes;
//     ulong dk_wmerge;
//     ulong dk_wmsec;
//     ulong dk_wkb;
//     ulong dk_xfers;
//     ulong dk_bsize;
//     ulong dk_time;
//     ulong dk_inflight;
//     ulong dk_backlog;
//     ulong dk_partition;
//     ulong dk_blocks;		/* in /proc/partitions only */
//     ulong dk_use;
//     ulong dk_aveq;
// };

// struct mem_stat {
//     long memtotal;
//     long memfree;
//     long memshared;
//     long buffers;
//     long cached;
//     long swapcached;
//     long active;
//     long inactive;
//     long hightotal;
//     long highfree;
//     long lowtotal;
//     long lowfree;
//     long swaptotal;
//     long swapfree;
// #ifndef SMALLMEM
//     long dirty;
//     long writeback;
//     long mapped;
//     long slab;
//     long committed_as;
//     long pagetables;
//     long hugetotal;
//     long hugefree;
//     long hugesize;
// #else
//     long bigfree;
// #endif /*SMALLMEM*/
// };

// struct vm_stat {
//     long long nr_dirty;
//     long long nr_writeback;
//     long long nr_unstable;
//     long long nr_page_table_pages;
//     long long nr_mapped;
//     long long nr_slab;
//     long long nr_slab_reclaimable;
//     long long nr_slab_unreclaimable;
//     long long pgpgin;
//     long long pgpgout;
//     long long pswpin;
//     long long pswpout;
//     long long pgalloc_high;
//     long long pgalloc_normal;
//     long long pgalloc_dma;
//     long long pgfree;
//     long long pgactivate;
//     long long pgdeactivate;
//     long long pgfault;
//     long long pgmajfault;
//     long long pgrefill_high;
//     long long pgrefill_normal;
//     long long pgrefill_dma;
//     long long pgsteal_high;
//     long long pgsteal_normal;
//     long long pgsteal_dma;
//     long long pgscan_kswapd_high;
//     long long pgscan_kswapd_normal;
//     long long pgscan_kswapd_dma;
//     long long pgscan_direct_high;
//     long long pgscan_direct_normal;
//     long long pgscan_direct_dma;
//     long long pginodesteal;
//     long long slabs_scanned;
//     long long kswapd_steal;
//     long long kswapd_inodesteal;
//     long long pageoutrun;
//     long long allocstall;
//     long long pgrotated;
// };

// #define NETMAX 32
// struct net_stat {
//     unsigned long if_name[17];
//     unsigned long long if_ibytes;
//     unsigned long long if_obytes;
//     unsigned long long if_ipackets;
//     unsigned long long if_opackets;
//     unsigned long if_ierrs;
//     unsigned long if_oerrs;
//     unsigned long if_idrop;
//     unsigned long if_ififo;
//     unsigned long if_iframe;
//     unsigned long if_odrop;
//     unsigned long if_ofifo;
//     unsigned long if_ocarrier;
//     unsigned long if_ocolls;
// };
// #ifdef PARTITIONS
// #define PARTMAX 256
// struct part_stat {
//     int part_major;
//     int part_minor;
//     unsigned long part_blocks;
//     char part_name[16];
//     unsigned long part_rio;
//     unsigned long part_rmerge;
//     unsigned long part_rsect;
//     unsigned long part_ruse;
//     unsigned long part_wio;
//     unsigned long part_wmerge;
//     unsigned long part_wsect;
//     unsigned long part_wuse;
//     unsigned long part_run;
//     unsigned long part_use;
//     unsigned long part_aveq;
// };


// struct data {
//     struct dsk_stat *dk;
//     struct cpu_stat cpu_total;
//     struct cpu_stat cpuN[CPUMAX];
//     struct mem_stat mem;
//     struct vm_stat vm;
//     struct nfs_stat nfs;
//     struct net_stat ifnets[NETMAX];
// #ifdef PARTITIONS
//     struct part_stat parts[PARTMAX];
// #endif /*PARTITIONS*/
//     struct timeval tv;
//     double time;
//     struct procsinfo *procs;

//     int proc_records;
//     int processes;
// } database[2], *p, *q;


// UARG,+Time,PID,ProgName,FullCommand
// UARG
// BBBP


function get_process_info() {


    //

// ps -eo pid,args 2>/dev/null

}

// 获取进程的完整参数
function get_process_args(pid) {

    var process_id = {
        "pid":0,
        "args":[]
    }
    // "ps -p %d -o args 2>/dev/null"
    


}

function get_stat() {


    // /proc/stat
// 得到cpu和进程等的统计信息

}


function get_vmstat() {


    // /proc/vmstat


}


function proc_diskstats() {
    
    // /proc/stat
    // disk_io 开头的行

    // /proc/diskstats

    // /proc/partitions

}

console.log(JSON.stringify(procs,undefined, 2));

export { procs }






// /* Lookup the right string */
// char *status(int n)
// {
//     switch (n) {
//     case 0:
// 	return "Run  ";
//     default:
// 	return "Sleep";
//     }
// }

// /* Lookup the right process state string */
// char *get_state(char n)
// {
//     static char duff[64];
//     switch (n) {
//     case 'R':
// 	return "Running  ";
//     case 'S':
// 	return "Sleeping ";
//     case 'D':
// 	return "DiskSleep";
//     case 'Z':
// 	return "Zombie   ";
//     case 'T':
// 	return "Traced   ";
//     case 'W':
// 	return "Paging   ";
//     default:
// 	snprintf(duff, 64, "%d", n);
// 	return duff;
//     }
// }

function    disk_group() {

    // #ifdef LSBLK_NO_TYPE
    // #define LSBLK_STRING "lsblk --nodeps --output NAME --noheadings | awk 'BEGIN {printf \"# This file created by: nmon -g auto\\n# It is an automatically generated disk-group file which excluses disk paritions\\n\" } { printf \"%s %s\\n\", $1, $1 }' >auto"
    // #else
    // #define LSBLK_STRING "lsblk --nodeps --output NAME,TYPE --raw | grep disk | awk 'BEGIN {printf \"# This file created by: nmon -g auto\\n# It is an automatically generated disk-group file which excluses disk paritions\\n\" } { printf \"%s %s\\n\", $1, $1 }' >auto"
    // #endif				/* LSBLK_NO_TYPE */


// BBBG,%03d,User Defined Disk Groups Name,Disks


// void list_dgroup(struct dsk_stat *dk)
// {
//     int i, j, k, n;
//     int first = 1;

//     /* DEBUG for (n = 0, i = 0; i < dgroup_total_groups; i++) {
//        fprintf(fp, "CCCG,%03d,%s", n++, dgroup_name[i]);
//        for (j = 0; j < dgroup_disks[i]; j++) {
//        if (dgroup_data[i*DGROUPITEMS+j] != -1) {
//        fprintf(fp, ",%d=%d", j, dgroup_data[i*DGROUPITEMS+j]);
//        }
//        }
//        fprintf(fp, "\n");
//        }
//      */
//     if (!show_dgroup)
// 	return;

//     for (n = 0, i = 0; i < dgroup_total_groups; i++) {
// 	if (first) {
// 	    fprintf(fp, "BBBG,%03d,User Defined Disk Groups Name,Disks\n",
// 		    n++);
// 	    first = 0;
// 	}
// 	fprintf(fp, "BBBG,%03d,%s", n++, dgroup_name[i]);
// 	for (k = 0, j = 0; j < dgroup_disks[i]; j++) {
// 	    if (dgroup_data[i * DGROUPITEMS + j] != -1) {
// 		fprintf(fp, ",%s",
// 			dk[dgroup_data[i * DGROUPITEMS + j]].dk_name);
// 		k++;
// 	    }
// 	    /* add extra line if we have lots to stop spreadsheet line width problems */
// 	    if (k == 128) {
// 		fprintf(fp, "\nBBBG,%03d,%s continued", n++,
// 			dgroup_name[i]);
// 	    }
// 	}
// 	fprintf(fp, "\n");
//     }
//     fprintf(fp, "DGBUSY,Disk Group Busy %s", hostname);
//     for (i = 0; i < DGROUPS; i++) {
// 	if (dgroup_name[i] != 0)
// 	    fprintf(fp, ",%s", dgroup_name[i]);
//     }
//     fprintf(fp, "\n");
//     fprintf(fp, "DGREAD,Disk Group Read KB/s %s", hostname);
//     for (i = 0; i < DGROUPS; i++) {
// 	if (dgroup_name[i] != 0)
// 	    fprintf(fp, ",%s", dgroup_name[i]);
//     }
//     fprintf(fp, "\n");
//     fprintf(fp, "DGWRITE,Disk Group Write KB/s %s", hostname);
//     for (i = 0; i < DGROUPS; i++) {
// 	if (dgroup_name[i] != 0)
// 	    fprintf(fp, ",%s", dgroup_name[i]);
//     }
//     fprintf(fp, "\n");
//     fprintf(fp, "DGSIZE,Disk Group Block Size KB %s", hostname);
//     for (i = 0; i < DGROUPS; i++) {
// 	if (dgroup_name[i] != 0)
// 	    fprintf(fp, ",%s", dgroup_name[i]);
//     }
//     fprintf(fp, "\n");
//     fprintf(fp, "DGXFER,Disk Group Transfers/s %s", hostname);
//     for (i = 0; i < DGROUPS; i++) {
// 	if (dgroup_name[i] != 0)
// 	    fprintf(fp, ",%s", dgroup_name[i]);
//     }
//     fprintf(fp, "\n");

//     /* If requested provide additional data available in /proc/diskstats */
//     if (extended_disk == 1 && disk_mode == DISK_MODE_DISKSTATS) {
// 	fprintf(fp, "DGREADS,Disk Group read/s %s", hostname);
// 	for (i = 0; i < DGROUPS; i++) {
// 	    if (dgroup_name[i] != 0)
// 		fprintf(fp, ",%s", dgroup_name[i]);
// 	}
// 	fprintf(fp, "\n");
// 	fprintf(fp, "DGREADMERGE,Disk Group merged read/s %s", hostname);
// 	for (i = 0; i < DGROUPS; i++) {
// 	    if (dgroup_name[i] != 0)
// 		fprintf(fp, ",%s", dgroup_name[i]);
// 	}
// 	fprintf(fp, "\n");
// 	fprintf(fp, "DGREADSERV,Disk Group read service time (SUM ms) %s",
// 		hostname);
// 	for (i = 0; i < DGROUPS; i++) {
// 	    if (dgroup_name[i] != 0)
// 		fprintf(fp, ",%s", dgroup_name[i]);
// 	}
// 	fprintf(fp, "\n");
// 	fprintf(fp, "DGWRITES,Disk Group write/s %s", hostname);
// 	for (i = 0; i < DGROUPS; i++) {
// 	    if (dgroup_name[i] != 0)
// 		fprintf(fp, ",%s", dgroup_name[i]);
// 	}
// 	fprintf(fp, "\n");
// 	fprintf(fp, "DGWRITEMERGE,Disk Group merged write/s %s", hostname);
// 	for (i = 0; i < DGROUPS; i++) {
// 	    if (dgroup_name[i] != 0)
// 		fprintf(fp, ",%s", dgroup_name[i]);
// 	}
// 	fprintf(fp, "\n");
// 	fprintf(fp,
// 		"DGWRITESERV,Disk Group write service time (SUM ms) %s",
// 		hostname);
// 	for (i = 0; i < DGROUPS; i++) {
// 	    if (dgroup_name[i] != 0)
// 		fprintf(fp, ",%s", dgroup_name[i]);
// 	}
// 	fprintf(fp, "\n");
// 	fprintf(fp, "DGINFLIGHT,Disk Group in flight IO %s", hostname);
// 	for (i = 0; i < DGROUPS; i++) {
// 	    if (dgroup_name[i] != 0)
// 		fprintf(fp, ",%s", dgroup_name[i]);
// 	}
// 	fprintf(fp, "\n");
// 	fprintf(fp, "DGBACKLOG,Disk Group Backlog time (ms) %s", hostname);
// 	for (i = 0; i < DGROUPS; i++) {
// 	    if (dgroup_name[i] != 0)
// 		fprintf(fp, ",%s", dgroup_name[i]);
// 	}
// 	fprintf(fp, "\n");
//     }
// }

}




















