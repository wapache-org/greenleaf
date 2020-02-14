/*pga4dash*/
SELECT
    pid,
    locktype,
    datname,
    relation::regclass,
    page,
    tuple,
    virtualxid
    transactionid,
    classid::regclass,
    objid,
    objsubid,
    virtualtransaction,
    mode,
    granted
FROM
    pg_locks l
    LEFT OUTER JOIN pg_database d ON (l.database = d.oid)
{{# if (d.did) { }}WHERE
    database = {{ d.did }}{{# } }} 
ORDER BY
    pid, locktype
;