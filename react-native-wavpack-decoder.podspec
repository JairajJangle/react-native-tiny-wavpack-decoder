require 'json'

package = JSON.parse(File.read(File.join(__dir__, 'package.json')))

Pod::Spec.new do |s|
  s.name         = 'react-native-wavpack-decoder'
  s.version      = package['version']
  s.summary      = package['description']
  s.homepage     = package['homepage']
  s.license      = package['license']
  s.authors      = package['author']

  s.platforms    = { :ios => '13.0' }
  s.source       = { :git => 'https://github.com/JairajJangle/react-native-tiny-wavpack-decoder.git', :tag => "v#{s.version}" }

  s.source_files = 'ios/**/*.{h,m,mm}', 'src/tiny-wavpack/**/*.{c,h}'
  s.public_header_files = 'ios/**/*.h', 'src/tiny-wavpack/common/*.h'
  s.requires_arc = true

  s.frameworks   = 'AVFoundation'

  s.pod_target_xcconfig = {
    'HEADER_SEARCH_PATHS' => '"$(PODS_ROOT)/../src/tiny-wavpack/common" "$(PODS_ROOT)/../src/tiny-wavpack/lib"',
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'CLANG_CXX_LIBRARY' => 'libc++'
  }

  # Use install_modules_dependencies for React Native 0.71+ compatibility
  if respond_to?(:install_modules_dependencies, true)
    install_modules_dependencies(s)
  else
    s.dependency 'React-Core'
    s.dependency 'ReactCommon/TurboModule'
  end
end