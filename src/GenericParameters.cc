#include "podio/GenericParameters.h"

#include <algorithm>

namespace podio {

template <typename T>
T getValFromMap(const GenericParameters::MapType<T>& map, const std::string& key) {
  const auto it = map.find(key);
  if (it == map.end()) {
    return T{};
  }
  const auto& iv = it->second;
  return iv[0];
}

// overload for string such that return by const ref is preferred
const std::string& getValFromMap(const GenericParameters::MapType<std::string>& map, const std::string& key) {
  static const std::string empty("");
  auto it = map.find(key);
  if (it == map.end()) {
    return empty;
  }
  const auto& iv = it->second;
  return iv[0];
}

int GenericParameters::getIntVal(const std::string& key) const {
  return getValue<int>(key);
}

float GenericParameters::getFloatVal(const std::string& key) const {
  return getValue<float>(key);
}

const std::string& GenericParameters::getStringVal(const std::string& key) const {
  return getValue<std::string>(key);
}

template <typename T>
std::vector<T>& fillValsFromMap(const GenericParameters::MapType<T>& map, const std::string& key,
                                std::vector<T>& values) {
  auto it = map.find(key);
  if (it != map.end()) {
    values.insert(values.end(), it->second.begin(), it->second.end());
  }
  return values;
}

IntVec& GenericParameters::getIntVals(const std::string& key, IntVec& values) const {
  return fillValsFromMap(_intMap, key, values);
}

FloatVec& GenericParameters::getFloatVals(const std::string& key, FloatVec& values) const {
  return fillValsFromMap(_floatMap, key, values);
}

StringVec& GenericParameters::getStringVals(const std::string& key, StringVec& values) const {
  return fillValsFromMap(_stringMap, key, values);
}

template <typename T>
const StringVec& getKeys(const GenericParameters::MapType<T>& map, StringVec& keys) {
  std::transform(map.begin(), map.end(), std::back_inserter(keys), [](auto const& pair) { return pair.first; });
  return keys;
}

const StringVec& GenericParameters::getIntKeys(StringVec& keys) const {
  return podio::getKeys(_intMap, keys);
}

const StringVec& GenericParameters::getFloatKeys(StringVec& keys) const {
  return podio::getKeys(_floatMap, keys);
}

const StringVec& GenericParameters::getStringKeys(StringVec& keys) const {
  return podio::getKeys(_stringMap, keys);
}

template <typename T>
int getStoredElements(const GenericParameters::MapType<T>& map, const std::string& key) {
  auto it = map.find(key);
  if (it == map.end()) {
    return 0;
  }
  return it->second.size();
}

int GenericParameters::getNInt(const std::string& key) const {
  return getN<int>(key);
}

int GenericParameters::getNFloat(const std::string& key) const {
  return getN<float>(key);
}

int GenericParameters::getNString(const std::string& key) const {
  return getN<std::string>(key);
}

void GenericParameters::setValues(const std::string& key, const IntVec& values) {
  setValue(key, values);
}

void GenericParameters::setValues(const std::string& key, const FloatVec& values) {
  setValue(key, values);
}

void GenericParameters::setValues(const std::string& key, const StringVec& values) {
  setValue(key, values);
}

} // namespace podio
