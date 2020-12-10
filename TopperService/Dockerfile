FROM mcr.microsoft.com/dotnet/aspnet:3.1 AS base
WORKDIR /app
EXPOSE 5000

ARG MSSQL_PASSWORD
ARG MSSQL_USER
ARG MSSQL_DB
ARG MSSQL_PORT=1433
ARG MSSQL_SERVER
ENV ConnectionStrings__default User ID=${MSSQL_USER};Password=${MSSQL_PASSWORD};Data Source=${MSSQL_SERVER},${MSSQL_PORT};Database=${MSSQL_DB};
ENV ASPNETCORE_URLS=http://*:5000

FROM mcr.microsoft.com/dotnet/sdk:3.1 AS build
WORKDIR /src
COPY ["TopperService/TopperService.csproj", "TopperService/"]
RUN dotnet restore "TopperService/TopperService.csproj"
COPY . .
WORKDIR "/src/TopperService"
RUN dotnet build "TopperService.csproj" -c Release -o /app/build

FROM build AS publish
RUN dotnet publish "TopperService.csproj" -c Release -o /app/publish

FROM base AS final
WORKDIR /app
COPY --from=publish /app/publish .
ENTRYPOINT ["dotnet", "TopperService.dll"]
