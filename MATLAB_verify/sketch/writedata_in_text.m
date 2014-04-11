load('back_36_hist_front.mat');
% load('back_6_hist_front.mat');
% load('back_36_hist_front.mat');

fid=fopen('back_36_hist_front.txt','a+');
fprintf(fid,'%g\r\n',hist_front);
fclose(fid);