import psutil
import os
import csv

process_name = "app" #change if necessary
process_pid = False;
this_pid = os.getpid();

data = []
header = ["pid", "process.status()", "cpu_percent", "memory_mb", "memory_percent", "cpu_times.user", "cpu_times.system", "system_memory.percent", "system_memory.total", "system_memory.available", "system_cpu_freq.current", "cpu01", "cpu02", "cpu03", "cpu04", "cpu05", "cpu06", "cpu07", "cpu08", "cpu09", "cpu10", "cpu11", "cpu12"]

def monitor_process(pid):
	try:
		system_cpu_percent = psutil.cpu_percent(interval=0.5, percpu=True)
		system_cpu_freq = psutil.cpu_freq();	
		system_memory = psutil.virtual_memory();
				
		process = psutil.Process(pid);			
		cpu_percent = process.cpu_percent(interval=0.5)/12; #cpu_count = 12
		memory_mb = process.memory_full_info().rss / (1024*1024);
		memory_percent = process.memory_percent();
		#time_consumed = sum(process.cpu_times()[:2]);
		cpu_times = process.cpu_times()

	#print("CPU [%]: {}, Memory [MB]: {}, Memory [%]: {}, Time [s]: {}".format(cpu_percent,memory_mb,memory_percent,time_consumed));
				
		#row = str(iterator) + "," + str(pid) + "," + str(process.status()) + "," + str(cpu_count) + ",";
		#row = row + str(cpu_percent) + "," + str(memory_mb) + "," + str(memory_percent) + ",";
		#row = row + ",".join(map(str,cpu_times))
		row = [pid, process.status(), cpu_percent, memory_mb, memory_percent, cpu_times.user, cpu_times.system, system_memory.percent, system_memory.total, system_memory.available, system_cpu_freq.current]	
		row.extend(system_cpu_percent)		
		return (row)
	except psutil.NoSuchProcess:
		return False
	except Exception as e:
		print(e)





while True:
	if process_pid == False:
		for proc in psutil.process_iter(['pid', 'name']):
			if (proc.info["name"] == process_name) and (proc.info["pid"] != this_pid):
					process_pid = proc.info["pid"];
					break
					
	if type(process_pid)!= bool:
		if psutil.pid_exists(process_pid):
			row = monitor_process(process_pid);
			if type(row) == list: 
				data.append(row);
			elif row == False:
				break
		else:
			break



with open('performance_sc1_r.csv', 'w', encoding='UTF8', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(header)
    writer.writerows(data)












