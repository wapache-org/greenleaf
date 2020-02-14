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
    granted,
    fastpath
FROM
    pg_locks l
    LEFT OUTER JOIN pg_database d ON (l.database = d.oid)
{{# if (d.did) { }}WHERE
    datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}){{# } }}
ORDER BY
    pid, locktype
;