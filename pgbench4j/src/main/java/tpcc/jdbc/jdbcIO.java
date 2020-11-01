/*
 * jdbcIO - execute JDBC statements
 *
 * Copyright (C) 2004-2016, Denis Lussier
 *
 */
package tpcc.jdbc;

import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Timestamp;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import tpcc.pojo.NewOrder;
import tpcc.pojo.Oorder;
import tpcc.pojo.OrderLine;

public class jdbcIO {

    private static final Logger log = LoggerFactory.getLogger(jdbcIO.class);

    public void insertOrder(PreparedStatement ordrPrepStmt, Oorder oorder) {

        try {

          ordrPrepStmt.setInt(1, oorder.o_id);
          ordrPrepStmt.setInt(2, oorder.o_w_id);
          ordrPrepStmt.setInt(3, oorder.o_d_id);
          ordrPrepStmt.setInt(4, oorder.o_c_id);
          ordrPrepStmt.setInt(5, oorder.o_carrier_id);
          ordrPrepStmt.setInt(6, oorder.o_ol_cnt);
          ordrPrepStmt.setInt(7, oorder.o_all_local);
          Timestamp entry_d = new java.sql.Timestamp(oorder.o_entry_d);
          ordrPrepStmt.setTimestamp(8, entry_d);

          ordrPrepStmt.addBatch();

      } catch(SQLException se) {
        log.error(se.getMessage());
      } catch (Exception e) {
        log.error(e.getMessage(), e);
      }

    }  // end insertOrder()

    public void insertNewOrder(PreparedStatement nworPrepStmt, NewOrder new_order) {

        try {
          nworPrepStmt.setInt(1, new_order.no_w_id);
          nworPrepStmt.setInt(2, new_order.no_d_id);
          nworPrepStmt.setInt(3, new_order.no_o_id);

          nworPrepStmt.addBatch();

      } catch(SQLException se) {
        log.error(se.getMessage());
      } catch (Exception e) {
        log.error(e.getMessage(), e);
       }

    }  // end insertNewOrder()

    public void insertOrderLine(PreparedStatement orlnPrepStmt, OrderLine order_line) {

      try {
        orlnPrepStmt.setInt(1, order_line.ol_w_id);
        orlnPrepStmt.setInt(2, order_line.ol_d_id);
        orlnPrepStmt.setInt(3, order_line.ol_o_id);
        orlnPrepStmt.setInt(4, order_line.ol_number);
        orlnPrepStmt.setLong(5, order_line.ol_i_id);

        Timestamp delivery_d = new Timestamp(order_line.ol_delivery_d);
        orlnPrepStmt.setTimestamp(6, delivery_d);

        orlnPrepStmt.setDouble(7, order_line.ol_amount);
        orlnPrepStmt.setLong(8, order_line.ol_supply_w_id);
        orlnPrepStmt.setDouble(9, order_line.ol_quantity);
        orlnPrepStmt.setString(10, order_line.ol_dist_info);

        orlnPrepStmt.addBatch();

      } catch(SQLException se) {
        log.error(se.getMessage());
      } catch (Exception e) {
          log.error(e.getMessage(), e);
      }

    }  // end insertOrderLine()

}  // end class jdbcIO()
