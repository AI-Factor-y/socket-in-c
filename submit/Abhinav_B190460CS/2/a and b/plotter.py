import matplotlib.pyplot as plt
import numpy as np

plt.style.use('_mpl-gallery')


def get_data(filename):


	with open(filename) as f:
		lines = f.readlines()


	data=[[],[]]

	for line in lines:

		curr_line=line[0:-2]
		
		x,y=curr_line.split(" ");
		x=float(x)
		y=float(y)
		data[0].append(x)
		data[1].append(y)

	return data


send_data=get_data("send_speed.txt");
rec_data=get_data("receive_speed.txt");

send_x=np.array(send_data[0])
send_y=np.array(send_data[1])

rec_x=np.array(rec_data[0])
rec_y=np.array(rec_data[1])


fig, ax = plt.subplots(figsize=(10, 6), layout='constrained')


ax.plot(send_x, send_y, linewidth=2.0)
ax.set_xlabel('time')
ax.set_ylabel("send speed")

plt.title("transmition send speed graph ")


plt.show()

fig, ax = plt.subplots(figsize=(10, 6), layout='constrained')

ax.plot(rec_x, rec_y, linewidth=2.0)
ax.set_xlabel('time')
ax.set_ylabel("receive_speed speed")

plt.title("transmition recieve speed graph")



plt.show();