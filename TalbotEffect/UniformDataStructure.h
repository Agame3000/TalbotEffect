#pragma once
#include <QVariant>
#include <QMetaType>
#include <QVector3D>
#include <QVector2D>

class VariantContainer : public QObject {
	Q_OBJECT
public:
	VariantContainer(int type);
	template<typename T>
	static VariantContainer* make() {
		return new VariantContainer(qMetaTypeId<T>());
	}

	const int type;

	template<typename T>
	void assign(T& value) {
		Q_ASSERT(qMetaTypeId<T>() == type);
		data.setValue(value);
		emit update();
	}

	template<typename T>
	VariantContainer& operator=(T &value) {
		assign(value);
		return *this;
	}

	template<typename T>
	void assign(const T& value) {
		Q_ASSERT(qMetaTypeId<T>() == type);
		data.setValue(value);
		emit update();
	}

	template<typename T>
	VariantContainer& operator=(const T& value) {
		assign(value);
		return *this;
	}

	template<typename T>
	T cast() {
		Q_ASSERT(qMetaTypeId<T>() == type);
		return data.value<T>();
	}

	QVariant* getData() {
		return &data;
	}

	template<typename T>
	operator T() {
		return cast<T>();
	}
protected:
	QVariant data;
signals:
	void update();
};

class MetadataComponent : public VariantContainer {
	Q_OBJECT
public:
	MetadataComponent(int type, void* defaultValue);
	template<typename T>
	static MetadataComponent* make(T defaultValue) {
		return new MetadataComponent(qMetaTypeId<T>(), &defaultValue);
	}
	template<typename T>
	static MetadataComponent* make(T defaultValue, std::vector<MetadataComponent*> &components) {
		MetadataComponent* c = make(defaultValue);
		components.push_back(c);
		return c;
	}

	using VariantContainer::operator=;
};

class DependedVariantContainer : public VariantContainer {
	Q_OBJECT
public:
	DependedVariantContainer(int type, std::function<void(VariantContainer&)> calcFunction, std::initializer_list<VariantContainer*> dependencies);
	template<typename T>
	static DependedVariantContainer* make(std::function<void(VariantContainer&)> calcFunction, std::initializer_list<VariantContainer*> dependencies) {
		return new DependedVariantContainer(qMetaTypeId<T>(), calcFunction, dependencies);
	}

	using VariantContainer::operator=;
private:
	std::function<void(VariantContainer&)> calcFunction;
public slots:
	void calculate();
};

class UniformDataComponent : public DependedVariantContainer {
	Q_OBJECT
public:
	UniformDataComponent(int type, const char* id, std::function<void(VariantContainer&)> calcFunction, std::initializer_list<VariantContainer*> dependencies);
	template<typename T>
	static UniformDataComponent* make(const char* id, std::function<void(VariantContainer&)> calcFunction, std::initializer_list<VariantContainer*> dependencies) {
		return new UniformDataComponent(qMetaTypeId<T>(), id, calcFunction, dependencies);
	}

	const char const* id;

	using DependedVariantContainer::operator=;
};

class UniformDataStructure {
public:
	void add(UniformDataComponent* component);
	void forEach(std::function<void(UniformDataComponent* c, int i)> action);
	const std::vector<UniformDataComponent*> const* getComps();
	UniformDataComponent* operator[](int i);

	~UniformDataStructure();
protected:
	std::vector<UniformDataComponent*> components;
};