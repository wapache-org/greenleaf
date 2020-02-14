/*pga4dash*/
SELECT
    gid,
    database,
    owner,
    transaction,
    to_char(prepared, 'YYYY-MM-DD HH24:MI:SS TZ') AS prepared
FROM
    pg_prepared_xacts
{{# if (d.did) { }}WHERE
    database = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}){{# } }}
ORDER BY
    gid, database, owner
;