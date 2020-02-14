
"use strict";
var view = "/*pga4dash*/\n";
  let add_union = false; 
 view+="\n";
  if (Array.contains(d.chart_names, 'session_stats')) { 
 view+="\n";
  add_union = true; 
 view+="\nSELECT 'session_stats' AS chart_name, row_to_json(t) AS chart_data\nFROM (SELECT\n (SELECT count(*) FROM pg_stat_activity";
  if (d.did) { 
 view+=" WHERE datname = (SELECT datname FROM pg_database WHERE oid = "+( d.did )+")";
  } 
 view+=" AS \""+( _('Total') )+"\",\n (SELECT count(*) FROM pg_stat_activity WHERE state = 'active'";
  if (d.did) { 
 view+=" AND datname = (SELECT datname FROM pg_database WHERE oid = "+( d.did )+")";
  } 
 view+=" AS \""+( _('Active') )+"\",\n (SELECT count(*) FROM pg_stat_activity WHERE state = 'idle'";
  if (d.did) { 
 view+=" AND datname = (SELECT datname FROM pg_database WHERE oid = "+( d.did )+")";
  } 
 view+=" AS \""+( _('Idle') )+"\"\n) t\n";
  } 
 view+=" /* end 'session_stats' */";
return view;