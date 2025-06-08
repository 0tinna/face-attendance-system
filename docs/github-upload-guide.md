# GitHub 仓库设置指南

本指南将帮助您将人脸考勤系统项目上传到GitHub。

## 前提条件

1. 创建 [GitHub](https://github.com/) 账号
2. 安装 [Git](https://git-scm.com/downloads)
3. 在本地设置 Git (如果尚未设置)

## 设置 Git

如果您是首次使用 Git，需要设置用户名和邮箱：

```bash
git config --global user.name "您的GitHub用户名"
git config --global user.email "您的邮箱地址"
```

## 上传项目到 GitHub 的步骤

### 1. 创建 GitHub 仓库

1. 登录您的 GitHub 账号
2. 点击右上角的 "+" 图标，选择 "New repository"
3. 填写仓库信息：
   - Repository name: 例如 `face-attendance-system`
   - Description: "基于Qt和SeetaFace的人脸考勤系统"
   - 选择 Public 或 Private
   - 不要勾选 "Initialize this repository with a README"
4. 点击 "Create repository"

### 2. 初始化本地 Git 仓库

打开命令行(终端)，进入项目根目录：

```bash
cd c:\D-APPLICATION\Qt_rk3568_opencv_Seetaface
```

初始化 Git 仓库：

```bash
git init
```

### 3. 添加远程仓库

添加刚才创建的 GitHub 仓库作为远程仓库：

```bash
git remote add origin https://github.com/0tinna/face-attendance-system.git
```

### 4. 添加文件并提交

添加所有文件到暂存区：

```bash
git add .
```

提交更改：

```bash
git commit -m "初始提交：人脸考勤系统"
```

### 5. 推送到 GitHub

```bash
git push -u origin master
```

如果您使用的是主分支名为 `main` 而不是 `master`，则使用：

```bash
git push -u origin main
```

### 6. 验证上传

访问 GitHub 仓库网址 `https://github.com/您的用户名/face-attendance-system`，确认所有文件已成功上传。

## 注意事项

1. `.gitignore` 文件已配置为排除不必要的文件，例如编译生成的文件和用户特定的设置文件。
2. 如果上传失败，请检查错误信息并解决相应问题。常见问题包括：
   - 远程仓库 URL 错误
   - 未授权 (需要登录)
   - 文件大小超限 (GitHub 限制单个文件大小不超过 100MB)
3. 如果文件太大，考虑使用 [Git LFS](https://git-lfs.github.com/) 或将其从版本控制中排除。

## 工作流建议

建议采用以下工作流进行开发：

1. 为新功能或修复创建新的分支
2. 在分支上开发和测试
3. 提交 Pull Request
4. 审查代码
5. 合并分支到主分支

## 其他有用的 Git 命令

- 查看状态：`git status`
- 查看变更：`git diff`
- 查看提交历史：`git log`
- 创建分支：`git checkout -b 分支名称`
- 切换分支：`git checkout 分支名称`
