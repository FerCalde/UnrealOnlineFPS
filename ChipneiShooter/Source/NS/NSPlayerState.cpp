
#include "Net/UnrealNetwork.h"
#include "NSPlayerState.h"

void ANSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANSPlayerState, m_fHealth)
}
