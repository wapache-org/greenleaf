

## collect_host_metrics

build/greenleaf -d data/sqlite.db -c conf/crontab.json -l debug

`conf/crontab.json` content: 

```json
[{
    "cron":"0/1 * * * * *",
    "name":"heartbeat",
    "description":"",
    "type":"action/js/shell/python",
    "mode":"main/thread/process",
    "pwd":"",
    "cmd":"",
    "args":[],
    "action":"quickjs_modules/linux/collect_host_metrics.js",
    "payload":""
}]

```
