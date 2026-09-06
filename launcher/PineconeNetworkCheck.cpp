// SPDX-License-Identifier: GPL-3.0-only
/*
 *  CloudClient - Minecraft Launcher
 *  Copyright (C) 2026 Octol1ttle <l1ttleofficial@outlook.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "CloudNetworkCheck.h"

#include "Application.h"
#include "net/HttpMetaCache.h"
#include "settings/SettingsObject.h"

CloudNetworkCheck::CloudNetworkCheck(QNetworkAccessManager* network)
{
    m_network = network;

    static auto s_urlToResult = std::array{
        std::pair{ QUrl("https://CloudClient.ru"), Result::UsePrimary },
        std::pair{ QUrl("https://CloudClient.github.io"), Result::UseNewFallback },
        std::pair{ QUrl("https://clouds-404.github.io"), Result::UseOldFallback },
    };
    for (auto& [url, result] : s_urlToResult) {
        launchRequest(url, result);
    }
}

QMap<CloudNetworkCheck::Result, QString> CloudNetworkCheck::metaUrls()
{
    return {
        { Result::UsePrimary, "" },
        { Result::UseNewFallback, "https://CloudClient.github.io/meta/v1/" },
        { Result::UseOldFallback, "https://clouds-404.github.io/meta/v1/" },
    };
}

QMap<CloudNetworkCheck::Result, QString> CloudNetworkCheck::fmlLibsUrls()
{
    return {
        { Result::UsePrimary, "" },
        { Result::UseNewFallback, "https://CloudClient.github.io/files/fmllibs/" },
        { Result::UseOldFallback, "https://clouds-404.github.io/files/fmllibs/" },
    };
}

QMap<CloudNetworkCheck::Result, QString> CloudNetworkCheck::newsUrls()
{
    return {
        { Result::UsePrimary, "" },
        { Result::UseNewFallback, "https://CloudClient.github.io/feed/feed.xml" },
        { Result::UseOldFallback, "https://clouds-404.github.io/feed/feed.xml" },
    };
}

QMap<CloudNetworkCheck::Result, QString> CloudNetworkCheck::translationsUrls()
{
    return {
        { Result::UsePrimary, "" },
        { Result::UseNewFallback, "https://CloudClient.github.io/i18n" },
        { Result::UseOldFallback, "https://clouds-404.github.io/i18n" },
    };
}

void CloudNetworkCheck::launchRequest(const QUrl& url, Result ifSuccess)
{
    QNetworkRequest request(url);
    request.setTransferTimeout(std::chrono::seconds(3));
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    auto* reply = m_network->head(request);
    m_pendingRequests++;

    qInfo() << "[CloudNetworkCheck] Checking" << url;
    connect(reply, &QNetworkReply::finished, this, [this, reply, ifSuccess] {
        m_pendingRequests--;

        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qInfo() << "[CloudNetworkCheck]" << reply->url() << "result:" << reply->error() << status;
        if (reply->error() == QNetworkReply::NoError && status < 400 && ifSuccess < m_result) {
            m_result = ifSuccess;
        }
        reply->deleteLater();

        if (!m_finished && (m_pendingRequests == 0 || m_result == Result::UsePrimary)) {
            qInfo() << "[CloudNetworkCheck] Final result:" << m_result;
            finished();
            m_finished = true;
        }
    });
}

bool CloudNetworkCheck::handleUrlOverride(const QString& overrideName, const QMap<Result, QString>& urlMap) const
{
    if (!urlMap.contains(m_result)) {
        return false;
    }
    const QString newOverride = urlMap.value(m_result);

    auto* settings = APPLICATION->settings();
    const auto currentOverride = settings->get(overrideName).toString();
    if (currentOverride == newOverride) {
        return false;
    }
    if (!currentOverride.isEmpty() && !urlMap.values().contains(currentOverride)) {
        return false;
    }

    settings->set(overrideName, newOverride);
    qInfo() << "[CloudNetworkCheck] Updated setting" << overrideName << "to" << newOverride;
    return true;
}

void CloudNetworkCheck::finished()
{
    if (!APPLICATION->settings()->get("CloudAutoServers").toBool()) {
        qInfo() << "[CloudNetworkCheck] Automatic server switching is disabled";
        return;
    }

    if (handleUrlOverride("MetaURLOverride", metaUrls())) {
        if (!APPLICATION->metacache()->softEvict()) {
            qWarning() << "[CloudNetworkCheck] Could not evict metacache during automatic meta switch";
        }
        APPLICATION->metacache()->SaveNow();
    }

    std::ignore = handleUrlOverride("LegacyFMLLibsURLOverride", fmlLibsUrls());

    if (const auto newsUrls = this->newsUrls(); newsUrls.contains(m_result)) {
        emit shouldReloadNews(newsUrls.value(m_result));
    }
}
