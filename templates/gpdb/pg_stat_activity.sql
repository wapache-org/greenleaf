{{# if (d.pset.dbversion >= 60000) { }}
select 
    sess_id as session_id,
    pid as process_id,

    client_addr,
    client_port,
    application_name,
    
    datid as database_oid,
    datname as database_name,
    usesysid as role_oid,
    usename as role_name,
    
    state,
    waiting,
    waiting_reason,

    backend_start,
    xact_start,
    query_start,
    EXTRACT(EPOCH FROM (now() - query_start)) as query_used,
    
    rsgid as resource_queue_oid,
    rsgname as resource_queue_name,
    rsgqueueduration as resource_queue_duration,

    query
from pg_stat_activity
where 1=1
{{# if (d.state) { }}
and state = '{{ d.state }}'
{{# } }}
;
{{# }else{ }}
select 
    sess_id as session_id,
    procpid as process_id,

    client_addr,
    client_port,
    application_name,
    
    datid as database_oid,
    datname as database_name,
    usesysid as role_oid,
    usename as role_name,

    case 
        when current_query='<IDLE>' then 'idle' 
        else 'active' 
    end as state,
    waiting,
    waiting_reason,

    backend_start,
    xact_start,
    query_start,
    EXTRACT(EPOCH FROM (now() - query_start)) as query_used,
    
    rsgid as resource_queue_oid,
    rsgname as resource_queue_name,
    rsgqueueduration as resource_queue_duration,

    current_query as query
from pg_stat_activity
where 1=1
{{# if (d.state==='active') { }}
and current_query != '<IDLE>'
{{# } }}
;
{{# } }}
