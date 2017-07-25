function varargout = serialRead(obj, varargin)
     size = deal(varargin{1});
     type = 0; 
     signed = 0; 
     size = floor(size);
     totalSize = size;
try 
   out = fread(obj, totalSize, type, signed); 
catch aException
   error(message('MATLAB:serial:fread:opfailed', aException.message));
end
data = out(1); 
varargout = cell(1,1);
try   
    data = double(data); 
    data = data + (data<0).*256;    
    varargout{1} = reshape(data, size, 1);   
catch    
end