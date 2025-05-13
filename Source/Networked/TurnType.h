#pragma once

UENUM(BlueprintType)
enum class ETurnType : uint8
{
	ETT_Left UMETA(DisplayName = "Turn Left"),
	ETT_Right UMETA(DisplayName = "Turn Right"),
	ETT_NoTurn UMETA(DisplayName = "No Turning"),

	ETT_MAX UMETA(DisplayName = "Default")
};