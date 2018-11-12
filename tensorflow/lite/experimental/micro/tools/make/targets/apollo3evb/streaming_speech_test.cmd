file ../../gen/apollo3evb_cortex-m4/bin/stream_micro_speech_test
target remote localhost:2331
load ../../gen/apollo3evb_cortex-m4/bin/stream_micro_speech_test
monitor reset
#break stream_main.c:155
#commands
#dump verilog value captured_data.txt captured_data
#end
#break stream_main.c:161 if ui32Status != 2
#break stream_main.c:286
break stream_main.c:294
commands
dump verilog value captured_data.txt captured_data
c
end
c
