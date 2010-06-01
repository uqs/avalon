close all
load('/home/krucker/Master_thesis/avalon/Simulation_data/data1.mat')
load('/home/krucker/Master_thesis/avalon/Simulation_data/ind.mat')
traj_plot=traj;
del_head=delta_head;
desired_head=180/pi*des_head;
head=180/pi*heading;
for i=2:ind
    file = ['/home/krucker/Master_thesis/avalon/Simulation_data/data', num2str(i), '.mat'];
    load(file)
    traj_plot = [traj_plot;traj];
    del_head = [del_head delta_head];
    desired_head = [desired_head 180/pi*des_head];
    head = [head 180/pi*heading];
end
figure(1)
plot(traj_plot(:,2), traj_plot(:,1),dest_y, dest_x,'rx-',wp_y_loc,wp_x_loc,'kx-',wp_y_loc(k),wp_x_loc(k),'ks',destinationy,destinationx,'sr')
axis([000 80000 -4000 8000])
figure(2)
t=[0:delta_t:(delta_t*(length(del_head)-1))];
plot(t,desired_head, t, head)%t,del_head)
legend('desired heading','true heading')
xlabel('t [s]')
ylabel('angle [°]')
