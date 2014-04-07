%该文件为批处理文件。把obj文件放到当前目录的data文件夹下面
%保存好的mat文件，就保存在当前目录下面。
%mat空间变量命名方式：data:点的坐标
%                   faces：三角面片的索引

%注意这个程序只能读取只包含点点之间的关系，不能含有有法线，纹理 等的信息

%%
%找到data文件下放的obj文件
rootDir = [pwd,'\','data'];%直接设置路径
rootDirInfo = dir(rootDir);
[rootDirNo, t] = size(rootDirInfo);%确定目录中有多少文件

%%
%对每一个obj文件进行读取，读入后，自动命名保存
for fileIndex = 3:rootDirNo
    fileName =  rootDirInfo(fileIndex).name
    l=length(fileName); %得到文件的名字
    tempname =fileName(1:(l-4));%得到.mat前面的名字
    name = [tempname,'.mat']
    filepath = [rootDir,'\',fileName]
    [data,faces] = readObj_v_f(filepath);
    save(name,'data','faces');
    clear ('data','faces'); 
end


