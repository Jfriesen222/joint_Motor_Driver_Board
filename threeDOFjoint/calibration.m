close all
x = [ones(6,1), ([360; 300; 240; 180; 120; 60] + 19)/1000];
y =[139 83 287 191 184 215;
    741 685 892 804 787 813;
    1340 1298 1511 1412 1425 1418;
    1953 1918 2127 2002 2032 2034;
    2577 2534 2727 2611 2649 2644;
    3176 3145 3333 3333 3225 3246]*-100;
for i = 1 : 6
    b1( : , i ) = x\y(:,i);
end

for i = 1 : 6
    b2( : , i ) = [ones(6,1), y(:,i)]\x(:,2);
end

figure
for i = 1:6
    subplot(2,3,i);
    scatter(x(:,2),y(:,i))
    yCalc = x*b(:,i);
    hold on
    plot(x(:,2),yCalc)
    title(num2str(i-1));
end