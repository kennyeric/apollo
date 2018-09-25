/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
//         #include "modules/perception/lib/config_manager/config_manager.h"
//
//         ConfigManager* config_manager = lib::Singleton<ConfigManager>::
//         get_instance();
//
//         string model_name = "FrameClassifier";
//         const ModelConfig* model_config = NULL;
//         if (!config_manager->GetModelConfig(model_name, &model_config)) {
//            LOG_ERROR << "not found model: " << model_name;
//            return false;
//         }
//
//         int int_value = 0;
//         if (!model_config->get_value("my_param_name", &int_value)) {
//             LOG_ERROR << "my_param_name not found."
//             return false;
//         }
//         using int_value....
//
//
// CONFIG FORMAT
//
// First you should define file: conf/config_manager.config,
// you can set the path by gflags
//    --config_manager_path=conf/config_manager.config
//
// file content like as:
// define all model config paths.
// ############################################
//
// model_config_path: "./conf/frame_classifier.config"
// model_config_path: "./conf/track_classifier.config"
//
// ############################################
//
// one line identify one model parameter config path.
// ModelConfig config file like as:
// file: ./conf/frame_classifier.config
// #############################################
//
// model_configs {
//     # FrameClassifier model.
//     name: "FrameClassifier"
//     version: "1.0.0"
//     integer_params {
//         name: "threshold1"
//         value: 1
//     }
//
//     integer_params {
//         name: "threshold2"
//         value: 2
//     }
//
//     string_params {
//         name: "threshold3"
//         value: "str3"
//     }
//
//     double_params {
//         name: "threshold4"
//         value: 4.0
//     }
//
//     array_integer_params {
//         name: "array_p1"
//         values: 1
//         values: 2
//         values: 3
//     }
//
//     array_string_params {
//         name: "array_p2"
//         values: "str1"
//         values: "str2"
//         values: "str3"
//         values: "str4"
//     }
//
//     array_string_params {
//         name: "array_p3"
//         values: "str1"
//         values: "str2"
//         values: "str3"
//         values: "str4"
//     }
//
//     array_double_params {
//         name: "array_p4"
//         values: 1.1
//         values: 1.2
//         values: 1.3
//         values: 1.4
//     }
// }
#ifndef MODULES_PERCEPTION_LIB_CONFIG_MANAGER_CONFIG_MANAGER_H_
#define MODULES_PERCEPTION_LIB_CONFIG_MANAGER_CONFIG_MANAGER_H_

#include <google/protobuf/message.h>

#include <map>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

#include "modules/perception/lib/singleton/singleton.h"
#include "modules/perception/lib/thread/mutex.h"

namespace apollo {
namespace perception {
namespace lib {

class ModelConfig;
class ModelConfigProto;

class ConfigManager {
 public:
  // thread-safe interface.
  bool Init();

  // thread-safe interface.
  bool Reset();

  bool GetModelConfig(const std::string &model_name,
                      const ModelConfig **model_config);

  size_t NumModels() const { return model_config_map_.size(); }

  const std::string &work_root() const { return work_root_; }

  const std::string &adu_data() const { return adu_data_; }

  void set_adu_data(const std::string &adu_data) { adu_data_ = adu_data; }

  void set_work_root(const std::string &work_root) { work_root_ = work_root; }

  ConfigManager(const ConfigManager &) = delete;
  ConfigManager operator=(const ConfigManager &) = delete;

 private:
  ConfigManager();
  ~ConfigManager();

  bool InitInternal();
  std::string get_env(const std::string &var_name);

  friend class lib::Singleton<ConfigManager>;

  typedef std::map<std::string, ModelConfig *> ModelConfigMap;
  typedef ModelConfigMap::iterator ModelConfigMapIterator;
  typedef ModelConfigMap::const_iterator ModelConfigMapConstIterator;

  // key: model_name
  ModelConfigMap model_config_map_;
  Mutex mutex_;  // multi-thread init safe.
  bool inited_ = false;
  std::string work_root_;  // ConfigManager work root dir.
  std::string adu_data_;
};

class ModelConfig {
 public:
  ModelConfig() {}
  ~ModelConfig() {}

  bool Reset(const ModelConfigProto &proto);

  std::string name() const { return name_; }

  bool get_value(const std::string &name, int *value) const {
    return get_value_from_map<int>(name, integer_param_map_, value);
  }

  bool get_value(const std::string &name, std::string *value) const {
    return get_value_from_map<std::string>(name, string_param_map_, value);
  }

  bool get_value(const std::string &name, double *value) const {
    return get_value_from_map<double>(name, double_param_map_, value);
  }

  bool get_value(const std::string &name, float *value) const {
    return get_value_from_map<float>(name, float_param_map_, value);
  }

  bool get_value(const std::string &name, bool *value) const {
    return get_value_from_map<bool>(name, bool_param_map_, value);
  }

  bool get_value(const std::string &name, std::vector<int> *values) const {
    return get_value_from_map<std::vector<int>>(name, array_integer_param_map_,
                                                values);
  }

  bool get_value(const std::string &name, std::vector<double> *values) const {
    return get_value_from_map<std::vector<double>>(
        name, array_double_param_map_, values);
  }

  bool get_value(const std::string &name, std::vector<float> *values) const {
    return get_value_from_map<std::vector<float>>(name, array_float_param_map_,
                                                  values);
  }

  bool get_value(const std::string &name,
                 std::vector<std::string> *values) const {
    return get_value_from_map<std::vector<std::string>>(
        name, array_string_param_map_, values);
  }

  bool get_value(const std::string &name, std::vector<bool> *values) const {
    return get_value_from_map<std::vector<bool>>(name, array_bool_param_map_,
                                                 values);
  }

  ModelConfig(const ModelConfig &) = delete;
  ModelConfig operator=(const ModelConfig &) = delete;

 private:
  template <typename T>
  bool get_value_from_map(const std::string &name,
                          const std::map<std::string, T> &container,
                          T *value) const;

  template <typename T>
  void RepeatedToVector(
      const google::protobuf::RepeatedField<T> &repeated_values,
      std::vector<T> *vec_values);

  std::string name_;
  std::string version_;

  typedef std::map<std::string, int> IntegerParamMap;
  typedef std::map<std::string, std::string> StringParamMap;
  typedef std::map<std::string, double> DoubleParamMap;
  typedef std::map<std::string, float> FloatParamMap;
  typedef std::map<std::string, bool> BoolParamMap;
  typedef std::map<std::string, std::vector<int>> ArrayIntegerParamMap;
  typedef std::map<std::string, std::vector<std::string>> ArrayStringParamMap;
  typedef std::map<std::string, std::vector<double>> ArrayDoubleParamMap;
  typedef std::map<std::string, std::vector<float>> ArrayFloatParamMap;
  typedef std::map<std::string, std::vector<bool>> ArrayBoolParamMap;

  IntegerParamMap integer_param_map_;
  StringParamMap string_param_map_;
  DoubleParamMap double_param_map_;
  FloatParamMap float_param_map_;
  BoolParamMap bool_param_map_;
  ArrayIntegerParamMap array_integer_param_map_;
  ArrayStringParamMap array_string_param_map_;
  ArrayDoubleParamMap array_double_param_map_;
  ArrayFloatParamMap array_float_param_map_;
  ArrayBoolParamMap array_bool_param_map_;
};

template <typename T>
bool ModelConfig::get_value_from_map(const std::string &name,
                                     const std::map<std::string, T> &container,
                                     T *value) const {
  typename std::map<std::string, T>::const_iterator citer =
      container.find(name);

  if (citer == container.end()) {
    return false;
  }

  *value = citer->second;
  return true;
}

template <typename T>
void ModelConfig::RepeatedToVector(
    const google::protobuf::RepeatedField<T> &repeated_values,
    std::vector<T> *vec_list) {
  vec_list->reserve(repeated_values.size());
  for (T value : repeated_values) {
    vec_list->push_back(value);
  }
}

class ConfigManagerError {
 public:
  explicit ConfigManagerError(const std::string &error_info)
      : error_info_(error_info) {}
  std::string What() const { return error_info_; }

 private:
  std::string error_info_;
};

template <typename T>
class ConfigRead {
 public:
  static T Read(const ModelConfig &config, const std::string &name) {
    T ret;
    if (!config.get_value(name, &ret)) {
      std::stringstream ss;
      ss << "Config name:" << config.name() << " read failed. "
         << "type:" << typeid(T).name() << " name:" << name;
      throw ConfigManagerError(ss.str());
    }
    return ret;
  }
};

template <typename T>
class ConfigRead<std::vector<T>> {
 public:
  static std::vector<T> Read(const ModelConfig &config,
                             const std::string &name) {
    std::vector<T> ret;
    if (!config.get_value(name, &ret)) {
      std::stringstream ss;
      ss << "Config name:" << config.name() << " read failed. "
         << "type:vector<" << typeid(T).name() << "> name:" << name;
      throw ConfigManagerError(ss.str());
    }
    return std::move(ret);
  }
};

}  // namespace lib
}  // namespace perception
}  // namespace apollo

#endif  // MODULES_PERCEPTION_LIB_CONFIG_MANAGER_CONFIG_MANAGER_H_