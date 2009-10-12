


file = fopen("config","r")

lines = file.readlines().split("\n")
variables = {}

#
# rudder_state 1 restart_rudder.sh
# navigator 10 restart_navigator.sh
# ...


class Variable:
	def __init__(self,name,timout,command):
		self.name = name
		self.timeout = timeout
		self.command = command
		self.var = store.variable(name)
		self.var.read()
		self.lasttimestamp = self.var.getTimestamp()
	
	def check(self):
		self.var.read()
		ts = self.var.getTimestamp()
		age = ts - self.lasttimestamp
		self.lasttimestamp = ts
		return age < self.timeout

	def run(self):
		os.system(self.command)


for line in lines:
	list = line.split(" ")
	variables[list[0]] = Variable(list[0],list[1],list[2])


while True:
	for var in variables.values():
		if not var.check():
			var.run()







