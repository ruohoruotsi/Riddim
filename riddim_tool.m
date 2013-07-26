function [ret] = riddim_tool(configFileStr)

%%% create gui and init structures
fig = riddim_gui;
riddim_model('init');

%%% read config m-file, why do we need this now? flexibility?
if nargin==1
  if exist(configFileStr)~=2
    warning('Config file does not exist');
  else
    eval(configFileStr);
  end
end


%%% read data structure and return data if expected
if nargout~=0 
  waitfor(fig);     % continue after gui is destroyed
  dataStruct  = riddim_model('return');

  if isempty(dataStruct)
    ret = [];
  else
    ret = dataStruct.my_entry;
  end
end

