figure(2)
close(2)
load('/home/krucker/Master_thesis/avalon/Simulation_data/data1.mat')
load('/home/krucker/Master_thesis/avalon/Simulation_data/ind.mat')
traj_plot=traj;
for i=2:ind
    file = ['/home/krucker/Master_thesis/avalon/Simulation_data/data', num2str(i), '.mat'];
    load(file)
    traj_plot = [traj_plot;traj];
end
figure(2)
plot(traj_plot(:,2), traj_plot(:,1),dest_y, dest_x,'rx-',wp_y_loc,wp_x_loc,'kx-',wp_y_loc(k),wp_x_loc(k),'ks',destinationy,destinationx,'sr')
axis([000 40000 -4000 8000])
