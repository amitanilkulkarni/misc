#!/bin/bash
#
# Amit Kulkarni (amitkul2@cisco.com)
#
# Usage:
#    $ ./pidgin_status.sh &
#
# Uses Firefox's recovery.js to periodically see if you're in
# a WebEx meeting and updates pidgin status accordingly.
#

RECOVERY_FILE_PATH="$(find ~/.mozilla -name "recovery.js" -printf "%T@ %p\n" | sort -n | cut -d' ' -f 2- | tail -n 1)"

OLD_STATUS="$(purple-remote getstatus)"
OLD_STATUSMSG="$(purple-remote getstatusmessage)"
WEBEX_ON=0


function setStatus {
    
    if [ "$(python2 <<< $'import json\nf = open('"\"$RECOVERY_FILE_PATH\""$', "r")\njdata = json.loads(f.read())\nf.close()\nfor win in jdata.get("windows"):\n\tfor tab in win.get("tabs"):\n\t\ti = tab.get("index") - 1\n\t\tprint tab.get("entries")[i].get("url")' | grep -c "cisco.*.webex.com")" == "0" ]
    then

	if [ $WEBEX_ON -eq 0 ]
	then	    
	    OLD_STATUS="$(purple-remote getstatus)"
	    OLD_STATUSMSG="$(purple-remote getstatusmessage)"
	else
	    WEBEX_ON=0
	    purple-remote "setstatus?status=${OLD_STATUS}&message=${OLD_STATUSMSG}"
	fi
	
    else
	WEBEX_ON=1
	purple-remote "setstatus?status=extended_away&message=In a WebEx meeting"
    fi

}


while true
do
    setStatus
    sleep 20
done
