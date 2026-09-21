from __future__ import annotations

import hashlib
import json
import os
import urllib.error
import urllib.request
from dataclasses import dataclass


class GitHubObservationError(RuntimeError):
    pass


@dataclass(frozen=True)
class GitHubObserver:
    repository: str = "SummaArs/herus"
    api_base: str = "https://api.github.com"

    def observe(self, ref: str = "main") -> dict:
        if self.repository != "SummaArs/herus":
            raise GitHubObservationError("REPO_NOT_ALLOWLISTED")
        if not ref or any(ch not in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._/-" for ch in ref):
            raise GitHubObservationError("INVALID_REF")
        token = os.environ.get("HERUS_GITHUB_TOKEN")
        headers = {
            "Accept": "application/vnd.github+json",
            "User-Agent": "HERUS-observer/1.0",
            "X-GitHub-Api-Version": "2022-11-28",
        }
        if token:
            headers["Authorization"] = f"Bearer {token}"
        repo = self._get(f"/repos/{self.repository}", headers)
        branch = self._get(f"/repos/{self.repository}/branches/{ref}", headers)
        commit = branch.get("commit", {})
        sha = commit.get("sha")
        if not isinstance(sha, str) or not sha:
            raise GitHubObservationError("MISSING_COMMIT_SHA")
        snapshot = {
            "repository_id": repo.get("id"),
            "full_name": repo.get("full_name"),
            "ref": ref,
            "commit_sha": sha,
            "default_branch": (repo.get("default_branch") or ""),
            "visibility": repo.get("visibility") or "unknown",
            "archived": bool(repo.get("archived")),
            "fork": bool(repo.get("fork")),
        }
        if snapshot["full_name"] != self.repository or snapshot["repository_id"] is None:
            raise GitHubObservationError("REPOSITORY_IDENTITY_MISMATCH")
        canonical = json.dumps(snapshot, ensure_ascii=False, sort_keys=True, separators=(",", ":")).encode("utf-8")
        return {"snapshot": snapshot, "snapshot_digest": hashlib.sha256(canonical).hexdigest()}

    def _get(self, path: str, headers: dict[str, str]) -> dict:
        if not path.startswith("/repos/SummaArs/herus/") and path != "/repos/SummaArs/herus":
            raise GitHubObservationError("ENDPOINT_NOT_ALLOWLISTED")
        request = urllib.request.Request(self.api_base + path, headers=headers, method="GET")
        try:
            with urllib.request.urlopen(request, timeout=10) as response:
                if response.status != 200:
                    raise GitHubObservationError(f"GITHUB_HTTP_{response.status}")
                payload = json.loads(response.read(2_000_000))
        except urllib.error.HTTPError as error:
            raise GitHubObservationError(f"GITHUB_HTTP_{error.code}") from error
        except (urllib.error.URLError, TimeoutError, json.JSONDecodeError) as error:
            raise GitHubObservationError("GITHUB_UNAVAILABLE") from error
        if not isinstance(payload, dict):
            raise GitHubObservationError("GITHUB_SCHEMA")
        return payload

    def assert_read_only(self, method: str) -> None:
        if method.upper() not in {"GET", "HEAD"}:
            raise GitHubObservationError("WRITE_METHOD_BLOCKED")
