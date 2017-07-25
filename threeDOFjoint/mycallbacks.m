function  mycallbacks(s,event)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
out = fread(s);
% x = sscanf(out, '%*s %ld %ld %ld %ld %ld %ld %*s %ld %ld %ld %ld %ld %ld %*s %ld %ld %ld %ld %ld %ld');
% b = [0.392050674843914,0.386470456114568,0.407298940012326,0.395656090962861,0.397477990316968,0.399684543222272;
%     9.85597470344030e-07,9.77773710739326e-07,9.83537409280089e-07,9.65842312157989e-07,9.81234655311406e-07,9.87567270331370e-07];
% if(length(x) == 19)
%     for ii = 1:6
%         x(ii) = (b(1,ii) + b(2,ii)*x(ii))*100;
%     end
%     x(7:12) =x(7:12) * 0.4165 * 0.224809 * 0.5;
%     for ii = 1:6
%         x(ii+12) = (b(1,ii) + b(2,ii)*x(ii+12))*100;
%     end
% end
disp(out');
%   A = sscanf(out, '%i');
%   current(j) = A(1);
end

