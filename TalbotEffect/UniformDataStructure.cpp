#include "UniformDataStructure.h"

VariantContainer::VariantContainer(int type) : type(type) {}

MetadataComponent::MetadataComponent(int type, void* defaultValue) : VariantContainer(type) {
	this->data.setValue(QVariant(type, defaultValue));
}

DependedVariantContainer::DependedVariantContainer(int type, std::function<void(VariantContainer&)> calcFunction, std::initializer_list<VariantContainer*> dependencies) : VariantContainer(type), calcFunction(calcFunction) {
	for (VariantContainer* c : dependencies)
		connect(c, &VariantContainer::update, this, &DependedVariantContainer::calculate);
}

void DependedVariantContainer::calculate() { calcFunction(*this); }

UniformDataComponent::UniformDataComponent(int type, const char* id, std::function<void(VariantContainer&)> calcFunction, std::initializer_list<VariantContainer*> dependencies) : DependedVariantContainer(type, calcFunction, dependencies), id(id) {}

void UniformDataStructure::add(UniformDataComponent* component) {
	components.push_back(component);
}

void UniformDataStructure::forEach(std::function<void(UniformDataComponent* c, int i)> action) {
	int i = 0;
	for (UniformDataComponent* c : components)
		action(c, i++);
}

const std::vector<UniformDataComponent*> const* UniformDataStructure::getComps() {
	return &components;
}

UniformDataComponent* UniformDataStructure::operator[](int i) {
	return components[i];
}

UniformDataStructure::~UniformDataStructure() {
	for (UniformDataComponent* c : components)
		delete c;
}