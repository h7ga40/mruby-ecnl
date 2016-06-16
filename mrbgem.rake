MRuby::Gem::Specification.new('mruby-ecnl') do |spec|
  spec.license = 'TOPPERS License'
  spec.author  = 'Cores Co., Ltd.'
  spec.summary = 'ECHONET Lite communication middleware'

  spec.cc.defines << %w(ECHONET_CONTROLLER_EXTENTION)
end
