import TranslationNode_vex as vextools

vex_file_name = 'ez015.vix'
vex = vextools.Vex(vex_file_name)

sched = vex['SCHED']
schedList = list(sched.iterkeys())
for scan in schedList:
    print "Scan:", scan
    scan_start = vextools.parse_vex_time(sched[scan]['start'])
    print scan_start
