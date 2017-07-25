function serialWrite(obj, varargin)
cmd = uint8(varargin{1});
mode = 0;
type = 5;
signed = 0;
try
    fwrite(obj, cmd, length(cmd), type, mode, signed);
catch aException
    error(message('MATLAB:serial:fwrite:opfailed', aException.message));
end