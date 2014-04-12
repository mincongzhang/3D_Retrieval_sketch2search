list = [2 5 6 8 9 12 14 16 18 26 36 37 57 64 69 76 77 82 87 88 102 124 128 140 142 148 164 168 188 195];

for i = 1:length(list)

    load(['back_',num2str(list(i)),'_hist_front.mat']);
    % load('back_6_hist_front.mat');
    % load('back_36_hist_front.mat');

    fid=fopen(['back_',num2str(i-1),'_hist_front.txt'],'a+');
    if(i~=length(list))
        fprintf(fid,'%g\r\n',hist_front);
    else
         fprintf(fid,'%g',hist_front);
    end
    fclose(fid);
end