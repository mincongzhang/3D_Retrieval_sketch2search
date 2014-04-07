
%调用格式：
%[skulldata,faces] = readObj_v_f();
%注意后面要添加引号

%%
%function [skulldata,faces] = readObj_v_f()

%用途：读取obj格式的点的信息。屏蔽了读取面的信息
%[FileName,PathName]=uigetfile('*.obj','Select the max obj');


%%
%10.05.07 修改
function [data,faces] = readObj_v_f(FileName)
%FileName 应该是全路径名字

[fid, message ] = fopen(FileName,'r');
 
if (fid == -1)
    disp(message); %打开失败的话，返回错误
    return;
end

 vertexCount = 0;
 faceCount = 0;
 while 1
    [prefix,count]=fscanf(fid,'%s',1);%1 代表只读一个数据
    switch prefix
        case '#'
            tline=fgetl(fid);
        case 'v'
            vertexCount=vertexCount+1;
            [data(vertexCount,:),count]=fscanf(fid,'%f',3);
       case 'f'
            faceCount=faceCount+1;
            [faces(faceCount,:),count]=fscanf(fid,'%d',4);
        otherwise
            tline=fgetl(fid);
    end
    if count==0 
        break;
    end
    %faceCount;
    %vertexCount;
end


fclose(fid);%关闭文件
fprintf('%s\n',FileName);
%fprintf('Finished!\n');%在主窗口显示
fprintf('%d\n',faceCount);%在主窗口显示face的个数
fprintf('%d\n',vertexCount);%在主窗口显示vertex的个数




end