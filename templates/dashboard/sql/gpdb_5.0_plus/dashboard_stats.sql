/*pga4dash*/
{{# let add_union = false; }}
{{# if (d.chart_names.includes('session_stats')) { }}
{{# add_union = true; }}
SELECT 'session_stats' AS chart_name, row_to_json(t) AS chart_data
FROM (SELECT
   (SELECT count(*) FROM pg_stat_activity{{# if (d.did) { }} WHERE datid = {{ d.did }} {{# } }}) AS "{{ _('Total') }}",
   (SELECT count(*) FROM pg_stat_activity WHERE current_query NOT LIKE '<IDLE>%'{{# if (d.did) { }} AND datid = {{ d.did }} {{# } }})  AS "{{ _('Active') }}",
   (SELECT count(*) FROM pg_stat_activity WHERE current_query LIKE '<IDLE>%'{{# if (d.did) { }} AND datid =  {{ d.did }} {{# } }})  AS "{{ _('Idle') }}"
) t
{{# } }}
{{# if (add_union && d.chart_names.includes('tps_stats')) { }}
UNION ALL
{{# } }}
{{# if (d.chart_names.includes('tps_stats')) { }}
{{# add_union = true; }}
SELECT 'tps_stats' AS chart_name, row_to_json(t) AS chart_data
FROM (SELECT
   (SELECT sum(xact_commit) + sum(xact_rollback) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Transactions') }}",
   (SELECT sum(xact_commit) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Commits') }}",
   (SELECT sum(xact_rollback) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Rollbacks') }}"
) t
{{# } }}
{{# if (add_union && d.chart_names.includes('ti_stats')) { }}
UNION ALL
{{# } }}
{{# if (d.chart_names.includes('ti_stats')) { }}
{{# add_union = true; }}
SELECT 'ti_stats' AS chart_name, row_to_json(t) AS chart_data
FROM (SELECT
   (SELECT sum(tup_inserted) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Inserts') }}",
   (SELECT sum(tup_updated) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Updates') }}",
   (SELECT sum(tup_deleted) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Deletes') }}"
) t
{{# } }}
{{# if (add_union && d.chart_names.includes('to_stats')) { }}
UNION ALL
{{# } }}
{{# if (d.chart_names.includes('to_stats')) { }}
{{# add_union = true; }}
SELECT 'to_stats' AS chart_name, row_to_json(t) AS chart_data
FROM (SELECT
   (SELECT sum(tup_fetched) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Fetched') }}",
   (SELECT sum(tup_returned) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Returned') }}"
) t
{{# } }}
{{# if (add_union && d.chart_names.includes('bio_stats')) { }}
UNION ALL
{{# } }}
{{# if (d.chart_names.includes('bio_stats')) { }}
{{# add_union = true; }}
SELECT 'bio_stats' AS chart_name, row_to_json(t) AS chart_data
FROM (SELECT
   (SELECT sum(blks_read) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Reads') }}",
   (SELECT sum(blks_hit) FROM pg_stat_database{{# if (d.did) { }} WHERE datname = (SELECT datname FROM pg_database WHERE oid = {{ d.did }}) {{# } }}) AS "{{ _('Hits') }}"
) t
{{# } }}
;