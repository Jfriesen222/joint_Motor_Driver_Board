classdef TensegrityPlot < handle
    properties
        nodePoints            % n by 3 matrix of node points
        stringNodes           %2 by ss matrix of node numbers for each string
        %end node, top row must be less than bottom row
        barNodes              %2 by bb matrix node numbers for each bar end
        %node, top row must be less than bottom row
        n                     %scalar number of nodes
        bb                    %scalar number of bars
        ss                    %scalar number of strings
        barRad                %bar radius for plotting
        stringRad             %string radius for plotting
        resolution
        sphereXX
        sphereYY
        sphereZZ
        cylXX
        cylYY
        cylZZ
        shapeHandle           %transform object for spheres which sit at nodes in plot
        memberTForms              %transform object for bar cylinders
        plotHandle
    end
    methods
        function obj = TensegrityPlot(nodePoints, stringNodes, barNodes, barRad, stringRad)
            if(size(nodePoints,2)~=3 || ~isnumeric(nodePoints))
                error('node points should be n by 3 matrix of doubles')
            end
            obj.nodePoints = nodePoints;
            obj.n = size(nodePoints,1);
            if(isscalar(barRad) && barRad>0)
                obj.barRad = barRad;
            else
                error('barRad requires positive scalar double value')
            end
            
            if(isscalar(stringRad) && stringRad>0)
                obj.stringRad = stringRad;
            else
                error('stringRad requires positive scalar double value')
            end
            
            
            %%%%%%%%%%%%%%% Check stringNodes for errors %%%%%%%%%%%%%%%%%
            if((isnumeric(stringNodes) && ~any(mod(stringNodes(:),1)))...
                    && size(stringNodes,1) == 2 )
                if  (max(stringNodes(:))<=obj.n) && (min(stringNodes(:))>0)
                    obj.ss = size(stringNodes,2);
                    for i= 1:obj.ss
                        if stringNodes(1,i) == stringNodes(2,i)
                            error('stringnodes has identical entries in a column')
                        else if stringNodes(1,i) > stringNodes(2,i)
                                stringNodes(1:2,i) = stringNodes(2:-1:1,i);
                            end
                        end
                    end
                    obj.stringNodes = stringNodes;
                    
                else
                    error('stringNodes entries need to be in the range of 1 to n')
                end
            else
                error('stringNodes should be a 2 by ss matrix of positive integers')
            end
            
            %%%%%%%%%%%%%%% Check barNodes for errors %%%%%%%%%%%%%%%%%
            
            obj.bb = size(barNodes,2);
            for i= 1:obj.bb
                if barNodes(1,i) == barNodes(2,i)
                    error('barnodes has identical entries in a column')
                else if barNodes(1,i) > barNodes(2,i)
                        barNodes(1:2,i) = barNodes(2:-1:1,i);
                    end
                end
            end
            obj.barNodes = barNodes;
            
            %%%%%%%%%%%%% Check for repeat bars or strings %%%%%%%%%%%%%
            B = unique([stringNodes barNodes]', 'rows');
            if size(B,1) ~= (obj.bb+obj.ss)
                error('SOme bars or strings are repeated between node sets')
            end
        end
        
        %%%%%%%%%%%%%%%%%%%% Plotting functions %%%%%%%%%%%%%%%%%%%%%%%%%%%
        function generatePlot(obj,ax)
            obj.resolution = 6;
            m = obj.resolution;
            obj.plotHandle = ax;
            [xx,yy,zz] = sphere(m);
            obj.sphereXX=xx*obj.barRad;
            obj.sphereYY=yy*obj.barRad;
            obj.sphereZZ=zz*obj.barRad;
            xxS = repmat([xx; NaN(1,(m+1))],obj.n,1);
            yyS = repmat([yy; NaN(1,(m+1))],obj.n,1);
            zzS =  repmat([zz; NaN(1,(m+1))],obj.n,1);
            for i = 1:obj.n
                xxS((1:(m+1))+(m+2)*(i-1),:) = xx + 3*(i-1);
            end
            [xx,yy,zz] = cylinder(ones(2,1),m);
            obj.cylXX=xx'*obj.barRad;
            obj.cylYY=yy'*obj.barRad;
            obj.cylZZ =zz';
            xxC = repmat([xx; NaN(1,(m+1))],obj.bb+obj.ss,1);
            yyC = repmat([yy; NaN(1,(m+1))],obj.bb+obj.ss,1);
            zzC = repmat([zz; NaN(1,(m+1))],obj.bb+obj.ss,1);
            yyC = yyC+3;
            for i = 1:(obj.bb+obj.ss)
                xxC((1:2)+3*(i-1),:) = xx + 3*(i-1);
            end
            
             obj.shapeHandle = surf(ax,[xxS; xxC],[yyS; yyC],[zzS; zzC],'LineStyle', 'none','CDataMode','Manual','Clipping','off');
             obj.shapeHandle.CData(1:((m+2)*obj.n+3*obj.bb),:) = 0;
             obj.shapeHandle.CData(((m+2)*obj.n+3*obj.bb + 1):end,:) = 1;
        end
        function updatePlot(obj)   
            m = obj.resolution;
            nn = obj.n;
            b = obj.bb;
            s = obj.ss;
            xx = obj.sphereXX;
            yy = obj.sphereYY;
            zz = obj.sphereZZ;
            xxx = NaN((m+2)*nn + 3*s+3*b,(m+1));
            yyy = NaN((m+2)*nn + 3*s+3*b,(m+1));
            zzz = NaN((m+2)*nn + 3*s+3*b,(m+1));
            for i =1:nn
                xxx((1:(m+1))+(m+2)*(i-1),:) = xx + obj.nodePoints(i,1);
                yyy((1:(m+1))+(m+2)*(i-1),:) = yy + obj.nodePoints(i,2);
                zzz((1:(m+1))+(m+2)*(i-1),:) = zz + obj.nodePoints(i,3);
            end           
            nodeXYZ1 = obj.nodePoints([obj.barNodes(1,:) obj.stringNodes(1,:)],:);
            nodeXYZ2 = obj.nodePoints([obj.barNodes(2,:) obj.stringNodes(2,:)],:);
            H = getTforms(nodeXYZ1, nodeXYZ2,b,obj.stringRad/obj.barRad);
            nodeXYZ1 = nodeXYZ1';
            xyzb = [obj.cylXX(:), obj.cylYY(:), obj.cylZZ(:)]';
            XYZ = [H*xyzb + repmat(nodeXYZ1(:),1,2*(m+1)), NaN(3*(b+s), (m+1))];
            X = XYZ(1:3:end,:)';
            Y = XYZ(2:3:end,:)';
            Z = XYZ(3:3:end,:)';
            xxx(((m+2)*(obj.n) + 1):((m+2)*nn + 3*s+3*b),:) = reshape(X(:),(m+1),3*(b+s))';
            yyy(((m+2)*(obj.n) + 1):((m+2)*nn + 3*s+3*b),:) = reshape(Y(:),(m+1),3*(b+s))';
            zzz(((m+2)*(obj.n) + 1):((m+2)*nn + 3*s+3*b),:) = reshape(Z(:),(m+1),3*(b+s))';
            obj.shapeHandle.XData = xxx;
            obj.shapeHandle.YData = yyy;
            obj.shapeHandle.ZData = zzz;
        end
        
    end
end

function [HH] = getTforms(p1,p2,b,bs_br)
n = size(p1,1);
length = sum((p2-p1).^2,2).^0.5;
lengthInv = ones(n,1)./length;
vec = (p2-p1).*lengthInv(:,[1 1 1]);
d =  vec(:,3);
isVert= 1:n;
isVert = isVert(d < -0.999999999);
isOtherVert= 1:n;
isOtherVert = isOtherVert(d > 0.9999999999);
s = ((1+d)*2).^0.5;
axis = vec(:,[2 1]);
axis(:,1) = -axis(:,1);
if(~isempty(isOtherVert))
    axis(isOtherVert,:) = repmat(sin(pi-0.001)*[1 0],size(isOtherVert,2),1);
    s(isOtherVert) = ones(size(isOtherVert,2),1);
end
if(~isempty(isVert))
    axis(isVert,:) = repmat([1 0],size(isVert,2),1);
    s(isVert) = 0.0000001*ones(size(isVert,2),1);
end
qin = [0.5*s, axis./s(:,[1 1])];
qinmag = sqrt(sum(qin.^2,2));
qin = qin./qinmag(:,[1 1 1]);
b2sScale = [ones(b,1); bs_br*ones(n-b,1)];
H1  = repmat(eye(3),1,1,n);
H1(1,1,:) = b2sScale.*(qin(:,1).^2 + qin(:,2).^2 - qin(:,3).^2);
H1(1,2,:) = 2.*b2sScale.*(qin(:,2).*qin(:,3));
H1(1,3,:) = 2.*b2sScale.*( - qin(:,1).*qin(:,3));
H1(2,1,:) = 2.*b2sScale.*(qin(:,2).*qin(:,3)) ;
H1(2,2,:) = b2sScale.*(qin(:,1).^2 - qin(:,2).^2 + qin(:,3).^2);
H1(2,3,:) = 2.*b2sScale.*( + qin(:,1).*qin(:,2));
H1(3,1,:) = 2*length.*( + qin(:,1).*qin(:,3));
H1(3,2,:) = 2*length.*( - qin(:,1).*qin(:,2));
H1(3,3,:) = length.*( qin(:,1).^2 - qin(:,2).^2 - qin(:,3).^2);
HH = reshape(H1,3,3*n)';
end
