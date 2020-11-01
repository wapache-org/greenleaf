/*
 * jTPCCConfig - Basic configuration parameters for jTPCC
 *
 * Copyright (C) 2003, Raul Barbosa
 * Copyright (C) 2004-2016, Denis Lussier
 * Copyright (C) 2016, Jan Wieck
 *
 */
package tpcc;

import java.text.*;

public interface jTPCCConfig
{
    String JTPCCVERSION = "4.1.1";

    int
    	/** 新订单 */
	    NEW_ORDER = 1, 
	    /** 支付 */
	    PAYMENT = 2, 
	    /** 订单状态 */
	    ORDER_STATUS = 3, 
	    /**  */
	    DELIVERY = 4, 
	    /**  */
	    STOCK_LEVEL = 5;

    String[] nameTokens = {"BAR", "OUGHT", "ABLE", "PRI", "PRES", "ESE", "ANTI", "CALLY", "ATION", "EING"};

    SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

    int  configCommitCount  = 10000;  // commit every n records in LoadData

    int  configWhseCount    = 10;
    int  configItemCount    = 100000; // tpc-c std = 100,000
    int  configDistPerWhse  = 10;     // tpc-c std = 10
    int  configCustPerDist  = 3000;   // tpc-c std = 3,000
}
