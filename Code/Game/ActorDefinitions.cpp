#include "Game/ActorDefinitions.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
//--------------------------------------------------------------------------------------------------------------------------------------------------------
std::vector<ActorDefinition*> ActorDefinition::s_actorDefinitions;
//--------------------------------------------------------------------------------------------------------------------------------------------------------
ActorDefinition::ActorDefinition()
{

}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void ActorDefinition::InitializeActorDefinitions(const char* path)
{
	XmlDocument actorDefsXml;
	actorDefsXml.LoadFile(path);
	XmlElement* rootElement = actorDefsXml.RootElement();
	XmlElement* actorDefinitionElement = rootElement->FirstChildElement();

	while (actorDefinitionElement)
	{
		ActorDefinition* actorDef = new ActorDefinition();
		actorDef->LoadFromXmlElement(*actorDefinitionElement);
		s_actorDefinitions.push_back(actorDef);
		actorDefinitionElement = actorDefinitionElement->NextSiblingElement();
	}
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void ActorDefinition::ClearDefinitions()
{
	for (ActorDefinition* actorDef : s_actorDefinitions)
	{
		delete actorDef;
	}
	s_actorDefinitions.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
ActorDefinition* const ActorDefinition::GetActorDef(const std::string& actorDefName)
{
	for (ActorDefinition* actorDef : s_actorDefinitions)
	{
		if (actorDef->m_name == actorDefName)
		{
			return actorDef;
		}
	}
	return nullptr;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------
bool ActorDefinition::LoadFromXmlElement(const XmlElement& xmlElement)
{
 	m_name = ParseXmlAttribute(xmlElement, "name", m_name);
	m_visible = ParseXmlAttribute(xmlElement, "visible", m_visible);
	m_health = ParseXmlAttribute(xmlElement, "health", m_health);
	m_corpseLifetime = ParseXmlAttribute(xmlElement, "corpseLifetime", m_corpseLifetime);
	m_faction = ParseXmlAttribute(xmlElement, "faction", m_faction);
	m_renderForward = ParseXmlAttribute(xmlElement, "renderForward", m_renderForward);
	m_solidColor = ParseXmlAttribute(xmlElement, "solidColor", m_solidColor);
	m_wireframeColor = ParseXmlAttribute(xmlElement, "wireframeColor", m_wireframeColor);
	m_canBePossessed = ParseXmlAttribute(xmlElement, "canBePossessed", m_canBePossessed);
	m_dieOnSpawn = ParseXmlAttribute(xmlElement, "dieOnSpawn", m_dieOnSpawn);

	// Parse child elements
	const XmlElement* childElement = xmlElement.FirstChildElement();
	while (childElement)
	{
		std::string elementName = childElement->Name();

		if (elementName == "Collision")
		{
			m_physicsRadius = ParseXmlAttribute(*childElement, "radius", m_physicsRadius);
			m_physicsHeight = ParseXmlAttribute(*childElement, "height", m_physicsHeight);
			m_collidesWithWorld = ParseXmlAttribute(*childElement, "collidesWithWorld", m_collidesWithWorld);
			m_collidesWithActors = ParseXmlAttribute(*childElement, "collidesWithActors", m_collidesWithActors);
			m_dieOnCollide = ParseXmlAttribute(*childElement, "dieOnCollide", m_dieOnCollide);
			m_damageOnCollide = ParseXmlAttribute(*childElement, "damageOnCollide", m_damageOnCollide);
			m_impulseOnCollide = ParseXmlAttribute(*childElement, "impulseOnCollide", m_impulseOnCollide);
		}
		if (elementName == "Physics")
		{
			m_simulated = ParseXmlAttribute(*childElement, "simulated", m_simulated);
			m_flying = ParseXmlAttribute(*childElement, "flying", m_flying);
			m_walkSpeed = ParseXmlAttribute(*childElement, "walkSpeed", m_walkSpeed);
			m_runSpeed = ParseXmlAttribute(*childElement, "runSpeed", m_runSpeed);
			m_drag = ParseXmlAttribute(*childElement, "drag", m_drag);
			m_turnSpeed = ParseXmlAttribute(*childElement, "turnSpeed", m_turnSpeed);
		}
		if (elementName == "Camera")
		{
			m_eyeHeight = ParseXmlAttribute(*childElement, "eyeHeight", m_eyeHeight);
			m_cameraFOVDegrees = ParseXmlAttribute(*childElement, "cameraFOV", m_cameraFOVDegrees);
		}
		if (elementName == "AI")
		{
			m_aiEnabled = ParseXmlAttribute(*childElement, "aiEnabled", m_aiEnabled);
			m_sightRadius = ParseXmlAttribute(*childElement, "sightRadius", m_sightRadius);
			m_sightAngle = ParseXmlAttribute(*childElement, "sightAngle", m_sightAngle);
		}
		if (elementName == "Inventory")
		{
			const XmlElement* weaponElement = childElement->FirstChildElement("Weapon");
			while (weaponElement)
			{
				std::string weaponName = ParseXmlAttribute(*weaponElement, "name", "");
				if (!weaponName.empty())
				{
					m_weapons.push_back(weaponName);
				}
				weaponElement = weaponElement->NextSiblingElement("Weapon");
			}
		}
		if (elementName == "Visuals")
		{
			m_actorSpriteSize = ParseXmlAttribute(*childElement, "size", m_actorSpriteSize);
			m_pivot = ParseXmlAttribute(*childElement, "pivot", m_pivot);
			m_renderLit = ParseXmlAttribute(*childElement, "renderLit", m_renderLit);
			m_renderRounded = ParseXmlAttribute(*childElement, "renderRounded", m_renderRounded);
			m_spriteSheetCellCount = ParseXmlAttribute(*childElement, "cellCount", m_spriteSheetCellCount);
			std::string shaderName = ParseXmlAttribute(*childElement, "shader", "");
			std::string spriteSheetPath = ParseXmlAttribute(*childElement, "spriteSheet", "");

			std::string billboardType = ParseXmlAttribute(*childElement, "billboardType", "");
			if (billboardType == "FullFacing")
			{
				m_billboardType = BillboardType::FULL_CAMERA_FACING;
			}
			if (billboardType == "FullOpposing")
			{
				m_billboardType = BillboardType::FULL_CAMERA_OPPOSING;
			}
			if (billboardType == "WorldUpFacing")
			{
				m_billboardType = BillboardType::WORLD_UP_CAMERA_FACING;
			}
			if (billboardType == "WorldUpOpposing")
			{
				m_billboardType = BillboardType::WORLD_UP_CAMERA_OPPOSING;
			}
			if (billboardType == "Custom")
			{
				m_billboardType = BillboardType::CUSTOM_PROJECTILE;
			}

			if (!spriteSheetPath.empty())
			{
				m_spriteSheetTexture = g_theRenderer->CreateOrGetTextureFromFile(spriteSheetPath.c_str());
				if (m_spriteSheetTexture == nullptr)
				{
					ERROR_AND_DIE("Failed to create or get texture after loading from xml");
				}

			}
			
			if (!shaderName.empty())
			{
				m_shader = g_theRenderer->CreateShaderOrGetFromFile((shaderName).c_str());
				if (m_shader == nullptr)
				{
					ERROR_AND_DIE("Failed to create shader after loading from xml");
				}
			}
			
			m_spriteSheet = new SpriteSheet(*m_spriteSheetTexture, m_spriteSheetCellCount);
			const XmlElement* animationGroupElement = childElement->FirstChildElement("AnimationGroup");
			while (animationGroupElement)
			{
				SpriteAnimationGroupDefinition animGroupDef;
				if (animGroupDef.LoadFromXmlElement(*animationGroupElement, m_spriteSheet))
				{
					m_animationGroupDefs.push_back(animGroupDef);
				}

				animationGroupElement = animationGroupElement->NextSiblingElement("AnimationGroup");
			}
		}
		if (elementName == "Sounds")
		{
			const XmlElement* soundElement = childElement->FirstChildElement("Sound");
			while (soundElement)
			{
				std::string soundName = ParseXmlAttribute(*soundElement, "sound", "");
				std::string soundPath = ParseXmlAttribute(*soundElement, "name", "");
				if (soundName == "Death")
				{
					m_deathSound = g_theAudio->CreateOrGetSound(soundPath);
				}
				if (soundName == "Hurt")
				{
					m_hurtSound = g_theAudio->CreateOrGetSound(soundPath);
				}
				soundElement = soundElement->NextSiblingElement("Sound");
			}
		}
		childElement = childElement->NextSiblingElement();
	}

	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
