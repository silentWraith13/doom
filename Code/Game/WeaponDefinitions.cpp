#include "Game/WeaponDefinitions.hpp"
#include "Game/GameCommon.hpp"
#include "Game/WeaponAnimGroupDefinition.hpp"
#include "Game/App.hpp"

//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::vector<WeaponDefinition*> WeaponDefinition::s_weaponDefinitions;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
WeaponDefinition::WeaponDefinition()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void WeaponDefinition::InitializeWeaponDefinitions(const char* path)
{
	XmlDocument weaponDefsXml;
	weaponDefsXml.LoadFile(path);
	XmlElement* rootElement = weaponDefsXml.RootElement();
	XmlElement* weaponDefinitionElement = rootElement->FirstChildElement();

	while (weaponDefinitionElement)
	{
		std::string elementName = weaponDefinitionElement->Name();
		WeaponDefinition* weaponDef = new WeaponDefinition();
		weaponDef->LoadFromXmlElement(*weaponDefinitionElement);
		s_weaponDefinitions.push_back(weaponDef);
		weaponDefinitionElement = weaponDefinitionElement->NextSiblingElement();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void WeaponDefinition::ClearDefinitions()
{
	for (WeaponDefinition* tileDef : s_weaponDefinitions)
	{
		delete tileDef;
	}
	s_weaponDefinitions.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
WeaponDefinition* const WeaponDefinition::GetWeaponDefinition(const std::string& weaponDefinitionName)
{
	for (WeaponDefinition* weaponDef : s_weaponDefinitions)
	{
		if (weaponDef->m_name == weaponDefinitionName)
		{
			return weaponDef;
		}
	}
	return nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool WeaponDefinition::LoadFromXmlElement(const XmlElement& xmlElement)
{
	m_name = ParseXmlAttribute(xmlElement, "name", m_name);
	m_refireTime = ParseXmlAttribute(xmlElement, "refireTime", m_refireTime);

	// Ray properties
	m_rayCount = ParseXmlAttribute(xmlElement, "rayCount", m_rayCount);
	m_rayCone = ParseXmlAttribute(xmlElement, "rayCone", m_rayCone);
	m_rayRange = ParseXmlAttribute(xmlElement, "rayRange", m_rayRange);
	m_rayDamage = ParseXmlAttribute(xmlElement, "rayDamage", m_rayDamage);
	m_rayImpulse = ParseXmlAttribute(xmlElement, "rayImpulse", m_rayImpulse);

	// Projectile properties
	m_projectileCount = ParseXmlAttribute(xmlElement, "projectileCount", m_projectileCount);
	m_projectileActor = ParseXmlAttribute(xmlElement, "projectileActor", m_projectileActor);
	m_projectileCone = ParseXmlAttribute(xmlElement, "projectileCone", m_projectileCone);
	m_projectileSpeed = ParseXmlAttribute(xmlElement, "projectileSpeed", m_projectileSpeed);

	// Melee properties
	m_meleeCount = ParseXmlAttribute(xmlElement, "meleeCount", m_meleeCount);
	m_meleeArc = ParseXmlAttribute(xmlElement, "meleeArc", m_meleeArc);
	m_meleeRange = ParseXmlAttribute(xmlElement, "meleeRange", m_meleeRange);
	m_meleeDamage = ParseXmlAttribute(xmlElement, "meleeDamage", m_meleeDamage);
	m_meleeImpulse = ParseXmlAttribute(xmlElement, "meleeImpulse", m_meleeImpulse);
	
	const XmlElement* childElement = xmlElement.FirstChildElement();
	while (childElement)
	{
		std::string elementName = childElement->Name();
		if (elementName == "HUD")
		{
			std::string shaderName = ParseXmlAttribute(*childElement, "shader", "");
			std::string baseTexturePath = ParseXmlAttribute(*childElement, "baseTexture", "");
			std::string reticleTexturePath = ParseXmlAttribute(*childElement, "reticleTexture", "");
			m_reticleSize = ParseXmlAttribute(*childElement, "reticleSize", m_reticleSize);
			m_spriteSize = ParseXmlAttribute(*childElement, "spriteSize", m_spriteSize);
			m_spritePivot = ParseXmlAttribute(*childElement, "spritePivot", m_spritePivot);
			if (!shaderName.empty())
			{
				m_shader = g_theRenderer->CreateShaderOrGetFromFile(shaderName.c_str());
				if (m_shader == nullptr)
				{
					ERROR_AND_DIE("Failed to create shader after loading from xml");
				}
			}

			if (!baseTexturePath.empty())
			{
				m_baseHUDTexture = g_theRenderer->CreateOrGetTextureFromFile(baseTexturePath.c_str());
				if (m_baseHUDTexture == nullptr)
				{
					ERROR_AND_DIE("Failed to create or get texture after loading from xml");
				}
			}

			if (!reticleTexturePath.empty())
			{
				m_reticleTexture = g_theRenderer->CreateOrGetTextureFromFile(reticleTexturePath.c_str());
				if (m_baseHUDTexture == nullptr)
				{
					ERROR_AND_DIE("Failed to create or get texture after loading from xml");
				}
			}

			const XmlElement* animationElement = childElement->FirstChildElement("Animation");
			while (animationElement)
			{
				WeaponAnimGroupDefinitions weaponGroupDef;
				if (weaponGroupDef.LoadFromXmlElement(*animationElement))
				{
					m_weaponAnimGroupDefs.push_back(weaponGroupDef);
				}
				animationElement = animationElement->NextSiblingElement("Animation");
			}

			const XmlElement* soundsElement = xmlElement.FirstChildElement("Sounds");
			if (soundsElement)
			{
				const XmlElement* soundElement = soundsElement->FirstChildElement("Sound");
				while (soundElement)
				{
  					m_sound = ParseXmlAttribute(*soundElement, "sound", "");
					std::string soundFilePath = ParseXmlAttribute(*soundElement, "name", "");

					if (!m_sound.empty())
					{
  						m_fireSound = g_theAudio->CreateOrGetSound(soundFilePath, true);
					}

					soundElement = soundElement->NextSiblingElement("Sound");
				}
			}
		}
		childElement = childElement->NextSiblingElement();
	}
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
