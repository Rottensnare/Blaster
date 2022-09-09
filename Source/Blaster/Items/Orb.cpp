


#include "Orb.h"

#include "Blaster/BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"


AOrb::AOrb()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	OrbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OrbMesh"));
	OrbMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OrbMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	OrbMesh->SetSimulatePhysics(false);
	SetRootComponent(OrbMesh);
	

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}


void AOrb::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AOrb::OnSphereOverlap);
	
}

void AOrb::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!HasAuthority()) return;
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		ABlasterPlayerState* BlasterPlayerState = Cast<ABlasterPlayerState>(BlasterCharacter->GetPlayerState());
		if(BlasterPlayerState)
		{
			if(BlasterPlayerState->GetTeam() != Team)
			{
				BlasterCharacter->ServerAttachOrb(this);
			}
		}
	}
}


