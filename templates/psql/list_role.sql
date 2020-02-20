/*

     , pg_catalog.pg_resgroupcapability, pg_catalog.pg_resourcetype
     pg_catalog.pg_resqueue, pg_catalog.pg_resqueue_attributes, pg_catalog.pg_resqueue_status, pg_catalog.pg_resqueuecapability, pg_catalog.pg_stat_resqueues

*/
{{# if (d.pset.sversion >= 80100) { }}
SELECT r.oid
     , r.rolname
     , r.rolsuper as "{{ _('Is Superuser') }}" 
     , r.rolinherit as "{{ _('Is Inheritance') }}" 
     , r.rolcreaterole as "{{ _('Create Role') }}" 
     , r.rolcreatedb as "{{ _('Create DB') }}" 
     , r.rolcanlogin as "{{ _('Can Login') }}" 
     , r.rolconnlimit as "{{ _('Connections Limit') }}"
     , r.rolvaliduntil as "{{ _('Password valid until ') }}"
{{# if (d.pset.dbtype === 'greenplum') { }}
     , r.rolcreaterexthttp as "{{ _('Create HTTP External Table') }}"
     , r.rolcreaterextgpfd as "{{ _('Create gpfdist Readable External Table') }}"
     , r.rolcreatewextgpfd as "{{ _('Create gpfdist Writable External Table') }}"
     , r.rolcreaterexthdfs as "{{ _('Create HDFS Readable External Table') }}"
     , r.rolcreatewexthdfs as "{{ _('Create HDFS Writable External Table') }}"
     , g.rsgname as "{{ _('Resource Group') }}" 
     , q.rsqname as "{{ _('Resource Queue') }}" 
{{# } }}
     , ARRAY(SELECT b.rolname
        FROM pg_catalog.pg_auth_members m
        JOIN pg_catalog.pg_roles b ON (m.roleid = b.oid)
        WHERE m.member = r.oid
       ) as "{{ _('Member Of') }}" 
{{# if (d.verbose && d.pset.sversion >= 80200) { }}
     , pg_catalog.shobj_description(r.oid, 'pg_authid') as "{{ _('Description') }}"
{{# } }}
{{# if (d.pset.sversion >= 90100) { }}
     , r.rolreplication as "{{ _('Replication') }}" 
{{# } }}
{{# if (d.pset.sversion >= 90500) { }}
     , r.rolbypassrls as "{{ _('Bypass RLS') }}" 
{{# } }}
FROM pg_catalog.pg_roles r
{{# if (d.pset.dbtype === 'greenplum') { }}
LEFT JOIN pg_catalog.pg_resgroup g ON r.rolresgroup = g.oid
LEFT JOIN pg_catalog.pg_resqueue q ON r.rolresqueue = q.oid
{{# } }}
{{# if (!d.showSystem && !d.pattern) { }}
WHERE r.rolname !~ '^pg_'
{{# } }}
{{# if (d.pattern) { }}
WHERE r.rolname !~ '{{ d.pattern }}'
{{# } }}

{{# } else { }}

SELECT u.usename AS rolname,
  u.usesuper AS rolsuper,
  true AS rolinherit, 
  false AS rolcreaterole,
  u.usecreatedb AS rolcreatedb, 
  true AS rolcanlogin,
  -1 AS rolconnlimit,
  u.valuntil as rolvaliduntil,
  ARRAY(SELECT g.groname FROM pg_catalog.pg_group g WHERE u.usesysid = ANY(g.grolist)) as memberof
FROM pg_catalog.pg_user u
{{# if (d.pattern) { }}
WHERE u.usename !~ '{{ d.pattern }}'
{{# } }}

{{# } }}
ORDER BY 2
;