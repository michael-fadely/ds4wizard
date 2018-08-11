#include "stdafx.h"
#include "Ds4LightOptions.h"

Ds4LightOptions::Ds4LightOptions(const Ds4Color& color)
	: color(color)
{
}

Ds4LightOptions::Ds4LightOptions(const Ds4LightOptions& other)
	: automaticColor(other.automaticColor), color(other.color), idleFade(other.idleFade)
{
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

void Ds4LightOptions::readJson(const nlohmann::json& json)
{
	automaticColor = json["automaticColor"];
	color          = fromJson<Ds4Color>(json["color"]);
	idleFade       = json["idleFade"];
}

void Ds4LightOptions::writeJson(nlohmann::json& json) const
{
	json["automaticColor"]  = automaticColor;
	json["color"]           = color.toJson();
	json["idleFade"]        = idleFade;
}
