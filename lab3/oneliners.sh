#!/bin/bash
# Run with bash oneliners.sh in the directory containing access.log.

# Q1. How many requests failed?
grep -c ' FAIL ' access.log

# Q2. How many different pages were requested, and which?
cut -d' ' -f5 access.log | sort -u | awk '{print} END {print "Different pages:", NR}'

# Q3. How many requests did each page get?
cut -d' ' -f5 access.log | sort | uniq -c

# Q4. Which user or users have the most failed requests, and how many?
grep ' FAIL ' access.log | cut -d' ' -f3 | sort | uniq -c | sort -k1,1nr -k2,2 | awk 'NR==1 {max=$1} $1==max {print $2 ": " $1 " failures"}'

# Q5. How many requests did user3 make, and how many failed?
awk '$3=="user3" {total++; if ($4=="FAIL") fails++} END {printf "user3: %d requests, %d failed\n", total, fails}' access.log

# Q6. Print the last 3 failed requests, showing only time and user.
grep ' FAIL ' access.log | tail -3 | cut -d' ' -f2,3

# Q7. Which login shells appear in /etc/passwd, and how many accounts use each?
cut -d: -f7 /etc/passwd | sort | uniq -c

# Q8. Compare a normal listing with a long listing.
ls /etc | wc -l; ls -l /etc | wc -l

# Bonus. Failed requests whose minute ends in 0, followed by their count.
grep -E '^2026-10-05 10:[0-5]0 user[0-6] FAIL ' access.log | tee /dev/stderr | wc -l
