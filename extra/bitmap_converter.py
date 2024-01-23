data = [
[0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,0,0],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,1,1],
[0,0,0,0,0,0,0,0]]

output = []

cur = 0
count = 0

for i in range(len(data[0])):
	for j in range(len(data)):
	    cur <<= 1
	    cur |= data[j][i]
	    count +=1
	    if count == 8:
	        output.append(format(cur, "#04x"))
	        cur = 0
	        count = 0
if cur != 0:
	output.append(format(cur, "#04X"))

print(*output, sep=", ")
print(len(output))