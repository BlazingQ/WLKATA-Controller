arm4

duration =  4016
cmd =  M20 G90 G00 X0 Y160 Z185 A0 B-25 C90 F2000
duration =  1021
cmd =  M20 G90 G00 X-10 Y198 Z165 A0 B-25 C90 F2000
duration =  1030
cmd =  M20 G91 G01 X0 Y0 Z-20 A0 B0 C0 F2000
duration =  1029
cmd =  M20 G91 G01 X0 Y0 Z20 A0 B0 C0 F2000
duration =  1021
cmd =  M20 G90 G00 X5 Y198 Z165 A0 B-25 C90 F2000
duration =  1029
cmd =  M20 G91 G01 X0 Y0 Z-20 A0 B0 C0 F2000
duration =  1030
cmd =  M20 G91 G01 X0 Y0 Z20 A0 B0 C0 F2000
duration =  1021
cmd =  M20 G90 G00 X20 Y198 Z165 A0 B-25 C90 F2000
duration =  1030
cmd =  M20 G91 G01 X0 Y0 Z-20 A0 B0 C0 F2000
duration =  1029
cmd =  M20 G91 G01 X0 Y0 Z20 A0 B0 C0 F2000
duration =  2012
cmd =  M21 G90 X0 Y0 Z0 A0 B0 C0 F2000



arm5
duration =  4016
cmd =  M20 G90 G00 X-15 Y173 Z150.4 A0 B0.1 C90 F2000
duration =  1022
cmd =  M20 G90 G00 X-10 Y173 Z106.1 A0 B0 C90 F2000
duration =  1022
cmd =  M20 G90 G00 X-15 Y173 Z115.4 A0 B0.1 C90 F2000
duration =  1021
cmd =  M20 G90 G00 X0.0999999 Y173 Z115.1 A0 B0.1 C90 F2000
duration =  1020
cmd =  M20 G90 G00 X10 Y173 Z108.8 A0 B0 C90 F2000
duration =  1022
cmd =  M20 G90 G00 X0.0999999 Y173 Z115.1 A0 B0.1 C90 F2000
duration =  1021
cmd =  M20 G90 G00 X25.1 Y173 Z114.8 A0 B0.1 C90 F2000
duration =  1021
cmd =  M20 G90 G00 X29.9 Y172.9 Z109.5 A0 B0.1 C90 F2000
duration =  1021
cmd =  M20 G90 G00 X25.1 Y178.6 Z114.8 A0 B0.1 C90 F2000
duration =  1020
cmd =  M20 G90 G00 X25 Y283.6 Z129.5 A0 B0 C270 F2000
duration =  1024
cmd =  M20 G90 G00 X25 Y283.5 Z116.2 A0 B0 C-90 F2000
duration =  1024
cmd =  M20 G90 G00 X25 Y283.4 Z115.9 A0 B0 C-90 F2000
duration =  1025
cmd =  M20 G90 G00 X5 Y283.6 Z114.2 A0 B0 C-90 F2000
duration =  1025
cmd =  M20 G90 G00 X9.9 Y283.5 Z116.2 A0 B0 C-90 F2000
duration =  1025
cmd =  M20 G90 G00 X5 Y283.6 Z115.9 A0 B0 C-90 F2000
duration =  1021
cmd =  M20 G90 G00 X-15 Y283.6 Z113.9 A0 B0 C-80 F2000
duration =  1021
cmd =  M20 G90 G00 X-9.9 Y283.5 Z112.2 A0 B0 C-79.9 F2000
duration =  1021
cmd =  M20 G90 G00 X198.67 Y0 Z230.72 A0 B0 C0 F2000


SLEEPPI
sleep 0 ms
[DEBUG]: [invokeSFBALGFunction] Calling completed
[TRACE]: [runSFBALG] Emit Event Output:SLEEPO
[TRACE]: [runResNetwork] Run In Composite FB:pi_arm2.EMS.app.Arm5 Internal FB:pi_arm2.EMS.app.Arm5.timer Event:SLEEPO Type:5
[TRACE]: [runCFBNetwork]Trigger FB:pi_arm2.EMS.app.Arm5 Internal FB:pi_arm2.EMS.app.Arm5.timer Type: 8 Event:SLEEPO Event Type:5
[TRACE]: [runCFBNetwork]Item:EventConnection From:pi_arm2.EMS.app.Arm5.timer/SLEEPO Type:8 To:pi_arm2.EMS.app.Arm5.arm5/VERIFY Type: 1
malloc(): invalid size (unsorted)
Aborted (core dumped)

