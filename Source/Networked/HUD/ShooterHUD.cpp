// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterHUD.h"

void AShooterHUD::DrawHUD()
{
	Super::DrawHUD();
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		float ScaledSpread = CrosshairSpreadMAX * HUDPack.CalcCrosshairSpread;

		if (HUDPack.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPack.CrosshairsCenter, ViewportCenter, Spread);
		}
		if (HUDPack.CrosshairsLeft)
		{
			FVector2D Spread(-ScaledSpread, 0.f);
			DrawCrosshair(HUDPack.CrosshairsLeft, ViewportCenter, Spread);
		}
		if (HUDPack.CrosshairsRight)
		{
			FVector2D Spread(ScaledSpread, 0.f);
			DrawCrosshair(HUDPack.CrosshairsRight, ViewportCenter, Spread);
		}
		if (HUDPack.CrosshairsTop)
		{
			FVector2D Spread(0.f, -ScaledSpread);
			DrawCrosshair(HUDPack.CrosshairsTop, ViewportCenter, Spread);
		}
		if (HUDPack.CrosshairsBottom)
		{
			FVector2D Spread(0.f, ScaledSpread);
			DrawCrosshair(HUDPack.CrosshairsBottom, ViewportCenter, Spread);
		}
	}
}

void AShooterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		Spread.X + ViewportCenter.X - (TextureWidth / 2.f),
		Spread.Y + ViewportCenter.Y - (TextureHeight / 2.f)
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		FLinearColor::White
	);
}

