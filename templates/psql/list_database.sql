/*

{
    pattern: null,
    verbose: false,
    pset: {
        sversion: 0
        dbtype: ''
    }
}

*/
SELECT d.oid as "{{ _('OID') }}"
     , d.datname as "{{ _('Name') }}"
     , pg_catalog.pg_get_userbyid(d.datdba) as "{{ _('Owner') }}"
     , pg_catalog.pg_encoding_to_char(d.encoding) as "{{ _('Encoding') }}"
{{# if (d.pset.dbtype === 'postgresql' && d.pset.sversion >= 80400) { }}
     , d.datcollate as "{{ _('Collate') }}"
     , d.datctype as "{{ _('Ctype') }}"
{{# } }}
     , d.datacl as "{{ _('AccessControl') }}"
{{# if (d.verbose && (d.pset.dbtype === 'greenplum' || d.pset.sversion >= 80400)) { }}
     , CASE WHEN pg_catalog.has_database_privilege(d.datname, 'CONNECT')
            THEN pg_catalog.pg_size_pretty(pg_catalog.pg_database_size(d.datname))
            ELSE '{{ _("No Access") }}'
       END as "{{ _('Size') }}"
{{# } }}
     , d.datconnlimit as "{{ _('ConnectionLimit') }}"
{{# if (d.verbose && d.pset.sversion >= 80000) { }}
     , t.spcname as "{{ _('Tablespace') }}"
{{# } }}
{{# if (d.verbose && d.pset.sversion >= 80200) { }}
     , pg_catalog.shobj_description(d.oid, 'pg_database') as "{{ _('Description') }}"
{{# } }}
FROM pg_catalog.pg_database d
{{# if (d.verbose && d.pset.sversion >= 80000) { }}
JOIN pg_catalog.pg_tablespace t on d.dattablespace = t.oid
{{# } }}
{{# if (d.pattern) { }}
WHERE d.datname SIMILAR TO '{{ d.pattern }}'
{{# } }}
ORDER BY 2
;