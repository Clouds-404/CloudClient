#include "ClaimAccount.h"
#include <launch/LaunchTask.h>

#include "Application.h"
#include "minecraft/auth/AccountList.h"

ClaimAccount::ClaimAccount(LaunchTask* parent, AuthSessionPtr session) : LaunchStep(parent)
{
    if (session->launchMode == LaunchMode::Normal) {
        auto accounts = APPLICATION->accounts();
        m_account = accounts->at(accounts->findAccountByProfileId(session->uuid));
    }
}

void ClaimAccount::executeTask()
{
    if (m_account) {
        lock.reset(new UseLock(m_account.get()));
    }
    emitSucceeded();
}

void ClaimAccount::finalize()
{
    lock.reset();
}
