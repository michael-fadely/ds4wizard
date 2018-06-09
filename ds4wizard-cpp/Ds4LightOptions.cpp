#include "stdafx.h"
#include "Ds4LightOptions.h"

Ds4LightOptions::Ds4LightOptions(const Ds4Color& color)
{
	this->color = color;
}

Ds4LightOptions::Ds4LightOptions(const Ds4LightOptions& other)
{
	color           = other.color;
	idleFade        = other.idleFade;
}

// TODO: replace with IComparable (if this were C#)
bool Ds4LightOptions::operator==(const Ds4LightOptions& other) const
{
	bool result = automaticColor == other.automaticColor;

	if (!automaticColor || !other.automaticColor)
	{
		result = result && color == other.color;
	}

	return result;
}

bool Ds4LightOptions::operator!=(const Ds4LightOptions& other) const
{
	return !(*this == other);
}

void Ds4LightOptions::readJson(const QJsonObject& json)
{
	automaticColor  = json["automaticColor"].toBool();
	color           = fromJson<Ds4Color>(json["color"].toObject());
	idleFade        = json["idleFade"].toBool();
}

void Ds4LightOptions::writeJson(QJsonObject& json) const
{
	json["automaticColor"]  = automaticColor;
	json["color"]           = color.toJson();
	json["idleFade"]        = idleFade;
}
