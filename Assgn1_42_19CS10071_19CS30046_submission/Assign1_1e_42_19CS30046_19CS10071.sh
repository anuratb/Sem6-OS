#!/bin/bash
V=false
while getopts "v" OPTION
do
  case $OPTION in
    v) V=true
       ;;
  esac
done
curl --header "User-Agent: Mozilla/5.0" https://www.example.com  >eg.html 

curl -i -4 --header "User-Agent: Mozilla/5.0" ip.jsontest.com
export REQ_HEADERS="Accept,Host,User-Agent"
 
curl  -4 --header "User-Agent: Mozilla/5.0" http://headers.jsontest.com | jq [".\"$( echo "$REQ_HEADERS" |sed -r "s/\,/\"\,\.\"/g" )\""]

($V) && echo '-------------------------Till before checking valid json-------------------------- '

rm -rf valid.txt invalid.txt
touch valid.txt
touch invalid.txt
for file in JSONData/*.json
do
    $(($V) && ((curl -v --header "User-Agent: Mozilla/5.0" -d "json=$(cat $file)" -X POST "http://validate.jsontest.com/") | jq ."validate")||((curl --header "User-Agent: Mozilla/5.0"  -d "json=$(cat $file)" -X POST "http://validate.jsontest.com/") | jq ."validate")) && ((echo $(basename $file))>>valid.txt) || ((echo $(basename $file))>>invalid.txt)
    

done

