#include <scene/jsonreader.h>
#include <scene/geometry/cube.h>
#include <scene/geometry/sphere.h>
#include <scene/geometry/cylinder.h>
#include <scene/geometry/mesh.h>
#include <scene/geometry/disc.h>
#include <scene/geometry/squareplane.h>
#include <scene/materials/mattematerial.h>
#include <scene/materials/mirrormaterial.h>
#include <scene/materials/transmissivematerial.h>
#include <scene/materials/glassmaterial.h>
#include <scene/materials/plasticmaterial.h>
#include <scene/mediums/homogeneousmedium.h>
#include <scene/lights/diffusearealight.h>
#include <scene/lights/spotlight.h>
#include <scene/lights/environmentlight.h>
#include <iostream>


void JSONReader::LoadSceneFromFile(QFile &file, const QStringRef &local_path, Scene &scene)
{
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray rawData = file.readAll();
        // Parse document
        QJsonDocument doc(QJsonDocument::fromJson(rawData));

        // Get JSON object
        QJsonObject json = doc.object();
        QJsonObject sceneObj, camera;
        QJsonArray primitiveList, materialList, mediumList, lightList, csgList;

        QMap<QString, std::shared_ptr<Material>> mtl_name_to_material;
        QMap<QString, std::shared_ptr<Medium>> med_name_to_material;
        QJsonArray frames = json["frames"].toArray();
        //check scene object for every frame
        foreach(const QJsonValue &frame, frames) {
            QJsonObject sceneObj = frame.toObject()["scene"].toObject();
            //load camera
            if(sceneObj.contains(QString("camera"))) {
                camera = sceneObj["camera"].toObject();
                scene.SetCamera(LoadCamera(camera));
            }
            //load all materials in QMap with mtl name as key and Material itself as value
            if(sceneObj.contains(QString("materials"))){
                materialList = sceneObj["materials"].toArray();
                foreach(const QJsonValue &materialVal, materialList){
                    QJsonObject materialObj = materialVal.toObject();
                    LoadMaterial(materialObj, local_path, &mtl_name_to_material);
                }
            }
            //load all mediums in QMap with med name as key and Medium itself as value
            if(sceneObj.contains(QString("mediums"))){
                mediumList = sceneObj["mediums"].toArray();
                foreach(const QJsonValue &mediumVal, mediumList){
                    QJsonObject mediumObj = mediumVal.toObject();
                    LoadMedium(mediumObj, local_path, &med_name_to_material);
                }
            }
            //load primitives and attach materials from QMap
            if(sceneObj.contains(QString("primitives"))) {
                primitiveList = sceneObj["primitives"].toArray();
                foreach(const QJsonValue &primitiveVal, primitiveList){
                    QJsonObject primitiveObj = primitiveVal.toObject();
                    LoadGeometry(primitiveObj, mtl_name_to_material, med_name_to_material, local_path, &scene.primitives, &scene.drawables);
                }
            }
            //load csg and attach materials from QMap
            if(sceneObj.contains(QString("csg"))) {
                csgList = sceneObj["csg"].toArray();
                foreach(const QJsonValue &csgVal, csgList){
                    QJsonObject csgObj = csgVal.toObject();
                    LoadCSG(csgObj, mtl_name_to_material, med_name_to_material, local_path, &scene.primitives, &scene.drawables);
                }
            }
            //load lights and attach materials from QMap
            if(sceneObj.contains(QString("lights"))) {
                lightList = sceneObj["lights"].toArray();
                foreach(const QJsonValue &lightVal, lightList){
                    QJsonObject lightObj = lightVal.toObject();
                    LoadLights(lightObj, mtl_name_to_material, med_name_to_material, local_path, &scene.primitives, &scene.lights, &scene.drawables);
                }
            }
        }
        scene.ComputeBounds();
        for(std::shared_ptr<Drawable> d : scene.drawables)
        {
            d->create();
        }
        file.close();
    }
}

bool JSONReader::LoadGeometry(QJsonObject &geometry, QMap<QString, std::shared_ptr<Material>> mtl_map, QMap<QString, std::shared_ptr<Medium>> med_map, const QStringRef &local_path, QList<std::shared_ptr<Primitive>> *primitives, QList<std::shared_ptr<Drawable>> *drawables)
{
    std::shared_ptr<Shape> shape = nullptr;
    //First check what type of geometry we're supposed to load
    QString type;
    if(geometry.contains(QString("shape"))){
        type = geometry["shape"].toString();
    }

    bool isMesh = false;
    if(QString::compare(type, QString("Mesh")) == 0)
    {
        //        shape = std::make_shared<Mesh>();
        auto mesh = std::make_shared<Mesh>();
        isMesh = true;

        Transform transform;
        if(geometry.contains(QString("transform"))) {
            QJsonObject qTransform = geometry["transform"].toObject();
            transform = LoadTransform(qTransform);
        }

        if(geometry.contains(QString("filename"))) {
            QString objFilePath = geometry["filename"].toString();
            std::static_pointer_cast<Mesh>(mesh)->LoadOBJ(QStringRef(&objFilePath), local_path, transform);
        }

        QString meshName("Unnamed Mesh");
        if(geometry.contains(QString("name"))) meshName = geometry["name"].toString();
        meshName.append(QString("'s Triangle"));
        for(auto triangle : mesh->faces)
        {
            auto primitive = std::make_shared<Primitive>(triangle);
            QMap<QString, std::shared_ptr<Material>>::iterator i;
            if(geometry.contains(QString("material"))) {
                QString material_name = geometry["material"].toString();
                for (i = mtl_map.begin(); i != mtl_map.end(); ++i) {
                    if(i.key() == material_name){
                        primitive->material = i.value();
                    }
                }
            }
            QMap<QString, std::shared_ptr<Medium>>::iterator im;
            if(geometry.contains(QString("medium"))) {
                QJsonArray medInterface =  geometry["medium"].toArray();
                QString med1 = medInterface.at(0).toString();
                QString med2 = medInterface.at(1).toString();
                std::shared_ptr<Medium> medium1 = nullptr;
                std::shared_ptr<Medium> medium2 = nullptr;
                for (im = med_map.begin(); im != med_map.end(); ++im) {
                    if(!med1.isEmpty() && im.key() == med1) medium1 = im.value();
                    if(!med2.isEmpty() && im.key() == med2) medium2 = im.value();
                }
                primitive->mediumInterface = std::make_shared<MediumInterface>(medium1, medium2);
            }
            primitive->name = meshName;
            (*primitives).append(primitive);
        }
        (*drawables).append(mesh);
    }
    else if(QString::compare(type, QString("Sphere")) == 0)
    {
        shape = std::make_shared<Sphere>();
    }
    else if(QString::compare(type, QString("Cylinder")) == 0)
    {
        shape = std::make_shared<Cylinder>();
    }
    else if(QString::compare(type, QString("SquarePlane")) == 0)
    {
        shape = std::make_shared<SquarePlane>();
    }
    else if(QString::compare(type, QString("Cube")) == 0)
    {
        shape = std::make_shared<Cube>();
    }
    else if(QString::compare(type, QString("Disc")) == 0)
    {
        shape = std::make_shared<Disc>();
    }
    else
    {
        std::cout << "Could not parse the geometry!" << std::endl;
        return NULL;
    }
    if(!isMesh)
    {
        // The Mesh class is handled differently
        // All Triangles are added to the Primitives list
        // but a single Drawable is created to render the Mesh
        auto primitive = std::make_shared<Primitive>(shape);
        QMap<QString, std::shared_ptr<Material>>::iterator i;
        if(geometry.contains(QString("material"))) {
            QString material_name = geometry["material"].toString();
            for (i = mtl_map.begin(); i != mtl_map.end(); ++i) {
                if(i.key() == material_name){
                    primitive->material = i.value();
                }
            }
        }
        QMap<QString, std::shared_ptr<Medium>>::iterator im;
        if(geometry.contains(QString("medium"))) {
            QJsonArray medInterface =  geometry["medium"].toArray();
            QString med1 = medInterface.at(0).toString();
            QString med2 = medInterface.at(1).toString();
            std::shared_ptr<Medium> medium1 = nullptr;
            std::shared_ptr<Medium> medium2 = nullptr;
            for (im = med_map.begin(); im != med_map.end(); ++im) {
                if(!med1.isEmpty() && im.key() == med1) medium1 = im.value();
                if(!med2.isEmpty() && im.key() == med2) medium2 = im.value();
            }
            primitive->mediumInterface = std::make_shared<MediumInterface>(medium1, medium2);
        }
        //load transform to shape
        if(geometry.contains(QString("transform"))) {
            QJsonObject transform = geometry["transform"].toObject();
            shape->transform = LoadTransform(transform);
        }
        if(geometry.contains(QString("name"))) primitive->name = geometry["name"].toString();
        (*primitives).append(primitive);
        (*drawables).append(primitive->shape);
    }
    return true;
}

bool JSONReader::LoadCSG(QJsonObject &csgObj, QMap<QString, std::shared_ptr<Material>> mtl_map, QMap<QString, std::shared_ptr<Medium>> med_map, const QStringRef &local_path, QList<std::shared_ptr<Primitive>> *primitives, QList<std::shared_ptr<Drawable>> *drawables)
{
    QJsonArray shapeList, transformList, operatorList, nameList, materialList, mediumList;
    // make operator array
    if(csgObj.contains(QString("operators"))) operatorList = csgObj["operators"].toArray();
    std::vector<operation> operators;
    for (int i = 0; i < operatorList.size(); i++) {
        QString oper_type = operatorList.at(i).toString();
        if (QString::compare(oper_type, QString("union")) == 0) {
            operators.push_back(UNION);
        }
        else if (QString::compare(oper_type, QString("difference")) == 0) {
            operators.push_back(DIFFER);
        }
        else if (QString::compare(oper_type, QString("intersection")) == 0) {
            operators.push_back(INTER);
        }
        else if (QString::compare(oper_type, QString("shape")) == 0) {
            operators.push_back(OBJECT);
        }
        else {
            std::cout << "Could not parse the geometry!" << std::endl;
            return NULL;
        }
    }
    // make transform array
    if(csgObj.contains(QString("transforms"))) transformList = csgObj["transforms"].toArray();
    // make name array
    if(csgObj.contains(QString("names"))) nameList = csgObj["names"].toArray();

    // make material array
    if(csgObj.contains(QString("materials"))) materialList = csgObj["materials"].toArray();
    QMap<QString, std::shared_ptr<Material>>::iterator iter;

    // make material array
    if(csgObj.contains(QString("mediums"))) mediumList = csgObj["mediums"].toArray();
    QMap<QString, std::shared_ptr<Medium>>::iterator im;

    // make shapes array
    std::vector<std::shared_ptr<Primitive>> prim_list;
    if(csgObj.contains(QString("shapes"))) {
        shapeList = csgObj["shapes"].toArray(); // array of QJsonValues
        for (int i = 0; i < shapeList.size(); i ++) {
            QJsonValue shapeVal = shapeList.at(i);
            std::shared_ptr<Shape> shape = nullptr;

            QString type = shapeVal.toString();

            if(QString::compare(type, QString("Sphere")) == 0)
            {
                shape = std::make_shared<Sphere>();
            }
            if(QString::compare(type, QString("Cylinder")) == 0)
            {
                shape = std::make_shared<Cylinder>();
            }
            else if(QString::compare(type, QString("SquarePlane")) == 0)
            {
                shape = std::make_shared<SquarePlane>();
            }
            else if(QString::compare(type, QString("Cube")) == 0)
            {
                shape = std::make_shared<Cube>();
            }
            else if(QString::compare(type, QString("Disc")) == 0)
            {
                shape = std::make_shared<Disc>();
            }
            else
            {
                std::cout << "Could not parse the geometry!" << std::endl;
                return NULL;
            }

            QJsonObject transform = transformList.at(i).toObject();
            shape->transform = LoadTransform(transform); // add transform

            auto primitive = std::make_shared<Primitive>(shape);

            QString name = nameList.at(i).toString(); // add name
            primitive->name = name;

            QString material_name = materialList.at(i).toString();
            for (iter = mtl_map.begin(); iter != mtl_map.end(); ++iter) {
                if(iter.key() == material_name){
                    primitive->material = iter.value();
                }
            }

            QJsonArray medInterface =  mediumList.at(i).toArray();
            QString med1 = medInterface.at(0).toString();
            QString med2 = medInterface.at(1).toString();
            std::shared_ptr<Medium> medium1 = nullptr;
            std::shared_ptr<Medium> medium2 = nullptr;
            for (im = med_map.begin(); im != med_map.end(); ++im) {
                if(!med1.isEmpty() && im.key() == med1) medium1 = im.value();
                if(!med2.isEmpty() && im.key() == med2) medium2 = im.value();
            }
            primitive->mediumInterface = std::make_shared<MediumInterface>(medium1, medium2);

            prim_list.push_back(primitive);
        }
    }

    std::shared_ptr<CSG> csg = std::make_shared<CSG>(prim_list, operators);

    if(csgObj.contains(QString("name"))) {
        csg->name = csgObj["name"].toString();
    }

    (*primitives).append(csg);
    for (int i = 0; i < csg->primitives.size(); i++) (*drawables).append(csg->primitives[i]->shape);

    return true;
}

bool JSONReader::LoadLights(QJsonObject &geometry, QMap<QString, std::shared_ptr<Material>> mtl_map, QMap<QString, std::shared_ptr<Medium>> med_map, const QStringRef &local_path, QList<std::shared_ptr<Primitive>> *primitives, QList<std::shared_ptr<Light>> *lights, QList<std::shared_ptr<Drawable> > *drawables)
{
    std::shared_ptr<Shape> shape = nullptr;
    //First check what type of geometry we're supposed to load
    QString type;
    if(geometry.contains(QString("shape"))){
        type = geometry["shape"].toString();

        if(QString::compare(type, QString("Mesh")) == 0)
        {
            Transform transform;
            //        shape = std::make_shared<Mesh>();
            auto mesh = std::make_shared<Mesh>();
            if(geometry.contains(QString("filename"))) {
                QString objFilePath = geometry["filename"].toString();
                std::static_pointer_cast<Mesh>(mesh)->LoadOBJ(QStringRef(&objFilePath), local_path, transform);
            }
        }
        else if(QString::compare(type, QString("Sphere")) == 0)
        {
            shape = std::make_shared<Sphere>();
        }
        else if(QString::compare(type, QString("Cylinder")) == 0)
        {
            shape = std::make_shared<Cylinder>();
        }
        else if(QString::compare(type, QString("SquarePlane")) == 0)
        {
            shape = std::make_shared<SquarePlane>();
        }
        else if(QString::compare(type, QString("Cube")) == 0)
        {
            shape = std::make_shared<Cube>();
        }
        else if(QString::compare(type, QString("Disc")) == 0)
        {
            shape = std::make_shared<Disc>();
        }
        else
        {
            std::cout << "Could not parse the geometry!" << std::endl;
            return NULL;
        }

        //load transform to shape
        if(geometry.contains(QString("transform"))) {
            QJsonObject transform = geometry["transform"].toObject();
            shape->transform = LoadTransform(transform);
        }


        (*drawables).append(shape);
    }

    //load light type
    std::shared_ptr<Light> lightType = nullptr;
    QString lgtType;
    if(geometry.contains(QString("type"))){
        lgtType = geometry["type"].toString();
    }

    if(QString::compare(lgtType, QString("DiffuseAreaLight")) == 0)
    {
        Color3f lightColor = ToVec3(geometry["lightColor"].toArray());
        Float intensity = static_cast< float >(geometry["intensity"].toDouble());
        bool twoSided = geometry.contains(QString("twoSided")) ? geometry["twoSided"].toBool() : false;
        lightType = std::make_shared<DiffuseAreaLight>(shape->transform, lightColor * intensity, shape, twoSided);
    }
    else if(QString::compare(lgtType, QString("SpotLight")) == 0)
    {
        Color3f lightColor = ToVec3(geometry["lightColor"].toArray());
        Float intensity = static_cast< float >(geometry["intensity"].toDouble());
        Float totalWidth = static_cast< float >(geometry["totalWidth"].toDouble());
        Float falloffStart = static_cast< float >(geometry["falloffStart"].toDouble());
        QJsonObject QJtransform = geometry["transform"].toObject();
        Transform transform = LoadTransform(QJtransform);
        lightType = std::make_shared<SpotLight>(transform, lightColor * intensity, totalWidth, falloffStart);
    }
    else if(QString::compare(lgtType, QString("EnvironmentLight")) == 0)
    {
        Color3f lightColor = ToVec3(geometry["lightColor"].toArray());
        Float intensity = static_cast< float >(geometry["intensity"].toDouble());
        QJsonObject QJtransform = geometry["transform"].toObject();
        Transform transform = LoadTransform(QJtransform);
        QString img_filepath = local_path.toString().append(geometry["map"].toString());
        std::shared_ptr<QImage> textureMap = std::make_shared<QImage>(img_filepath);
        lightType = std::make_shared<EnvironmentLight>(transform, lightColor * intensity, textureMap);
    }
    else
    {
        std::cout << "Could not parse the geometry!" << std::endl;
        return NULL;
    }


    auto primitive = std::make_shared<Primitive>(shape, nullptr,  std::static_pointer_cast<Light>(lightType)); //TODO
    QMap<QString, std::shared_ptr<Material>>::iterator i;
    if(geometry.contains(QString("material"))) {
        QString material_name = geometry["material"].toString();
        for (i = mtl_map.begin(); i != mtl_map.end(); ++i) {
            if(i.key() == material_name){
                primitive->material = i.value();
            }
        }
    }
    QMap<QString, std::shared_ptr<Medium>>::iterator im;
    if(geometry.contains(QString("medium"))) {
        QJsonArray medInterface =  geometry["medium"].toArray();
        QString med1 = medInterface.at(0).toString();
        QString med2 = medInterface.at(1).toString();
        std::shared_ptr<Medium> medium1 = nullptr;
        std::shared_ptr<Medium> medium2 = nullptr;
        for (im = med_map.begin(); im != med_map.end(); ++im) {
            if(!med1.isEmpty() && im.key() == med1) medium1 = im.value();
            if(!med2.isEmpty() && im.key() == med2) medium2 = im.value();
        }
        primitive->mediumInterface = std::make_shared<MediumInterface>(medium1, medium2);
    }

    if(geometry.contains(QString("name")))
    {
        primitive->name = geometry["name"].toString();
        lightType->name = geometry["name"].toString();
    }

    (*primitives).append(primitive);
    (*lights).append(lightType);
    return true;
}

bool JSONReader::LoadMaterial(QJsonObject &material, const QStringRef &local_path, QMap<QString, std::shared_ptr<Material> > *mtl_map)
{
    QString type;

    //First check what type of material we're supposed to load
    if(material.contains(QString("type"))) type = material["type"].toString();

    if(QString::compare(type, QString("MatteMaterial")) == 0)
    {
        std::shared_ptr<QImage> textureMap;
        std::shared_ptr<QImage> normalMap;
        Color3f Kd = ToVec3(material["Kd"].toArray());
        Float sigma = static_cast< float >(material["sigma"].toDouble());
        if(material.contains(QString("textureMap"))) {
            QString img_filepath = local_path.toString().append(material["textureMap"].toString());
            textureMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<MatteMaterial>(Kd, sigma, textureMap, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else if(QString::compare(type, QString("MirrorMaterial")) == 0)
    {
        std::shared_ptr<QImage> roughnessMap;
        std::shared_ptr<QImage> textureMap;
        std::shared_ptr<QImage> normalMap;
        Color3f Kr = ToVec3(material["Kr"].toArray());
        float roughness = 0.f;
        if(material.contains(QString("roughness"))) {
            roughness = material["roughness"].toDouble();
        }
        if(material.contains(QString("roughnessMap"))) {
            QString img_filepath = local_path.toString().append(material["roughnessMap"].toString());
            roughnessMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("textureMap"))) {
            QString img_filepath = local_path.toString().append(material["textureMap"].toString());
            textureMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<MirrorMaterial>(Kr, roughness, roughnessMap, textureMap, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else if(QString::compare(type, QString("TransmissiveMaterial")) == 0)
    {
        std::shared_ptr<QImage> textureMap;
        std::shared_ptr<QImage> normalMap;
        Color3f Kt = ToVec3(material["Kt"].toArray());
        float eta = material["eta"].toDouble();
        if(material.contains(QString("textureMap"))) {
            QString img_filepath = local_path.toString().append(material["textureMap"].toString());
            textureMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<TransmissiveMaterial>(Kt, eta, textureMap, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else if(QString::compare(type, QString("GlassMaterial")) == 0)
    {
        std::shared_ptr<QImage> textureMapRefl;
        std::shared_ptr<QImage> textureMapTransmit;
        std::shared_ptr<QImage> normalMap;
        Color3f Kr = ToVec3(material["Kr"].toArray());
        Color3f Kt = ToVec3(material["Kt"].toArray());
        float eta = material["eta"].toDouble();
        if(material.contains(QString("textureMapRefl"))) {
            QString img_filepath = local_path.toString().append(material["textureMapRefl"].toString());
            textureMapRefl = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("textureMapTransmit"))) {
            QString img_filepath = local_path.toString().append(material["textureMapTransmit"].toString());
            textureMapTransmit = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<GlassMaterial>(Kr, Kt, eta, textureMapRefl, textureMapTransmit, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else if(QString::compare(type, QString("PlasticMaterial")) == 0)
    {
        std::shared_ptr<QImage> roughnessMap;
        std::shared_ptr<QImage> textureMapDiffuse;
        std::shared_ptr<QImage> textureMapSpecular;
        std::shared_ptr<QImage> normalMap;
        Color3f Kd = ToVec3(material["Kd"].toArray());
        Color3f Ks = ToVec3(material["Ks"].toArray());
        float roughness = material["roughness"].toDouble();
        if(material.contains(QString("roughnessMap"))) {
            QString img_filepath = local_path.toString().append(material["roughnessMap"].toString());
            roughnessMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("textureMapDiffuse"))) {
            QString img_filepath = local_path.toString().append(material["textureMapDiffuse"].toString());
            textureMapDiffuse = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("textureMapSpecular"))) {
            QString img_filepath = local_path.toString().append(material["textureMapSpecular"].toString());
            textureMapSpecular = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<PlasticMaterial>(Kd, Ks, roughness, roughnessMap, textureMapDiffuse, textureMapSpecular, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else
    {
        std::cout << "Could not parse the material!" << std::endl;
        return false;
    }

    return true;
}

bool JSONReader::LoadMedium(QJsonObject &medium, const QStringRef &local_path, QMap<QString, std::shared_ptr<Medium> > *med_map)
{
    QString type;

    //First check what type of material we're supposed to load
    if(medium.contains(QString("type"))) type = medium["type"].toString();

    if(QString::compare(type, QString("HomogeneousMedium")) == 0)
    {
        float density = static_cast< float >(medium["density"].toDouble());
        float sigma_a = static_cast< float >(medium["sigma_a"].toDouble());
        float sigma_s = static_cast< float >(medium["sigma_s"].toDouble());
        float g = static_cast< float >(medium["g"].toDouble());
        auto result = std::make_shared<HomogeneousMedium>(sigma_a, sigma_s, g, density);
        med_map->insert(medium["name"].toString(), result);
    }
    else
    {
        std::cout << "Could not parse the medium!" << std::endl;
        return false;
    }

    return true;
}

Camera JSONReader::LoadCamera(QJsonObject& camera)
{
    Camera result;
    if(camera.contains(QString("target"))) result.ref = ToVec3(camera["target"].toArray());
    if(camera.contains(QString("eye"))) result.eye = ToVec3(camera["eye"].toArray());
    if(camera.contains(QString("worldUp"))) result.world_up = ToVec3(camera["worldUp"].toArray());
    if(camera.contains(QString("width"))) result.width = camera["width"].toDouble();
    if(camera.contains(QString("height"))) result.height = camera["height"].toDouble();
    if(camera.contains(QString("fov"))) result.fovy = camera["fov"].toDouble();
    if(camera.contains(QString("nearClip"))) result.near_clip = camera["nearClip"].toDouble();
    if(camera.contains(QString("farClip"))) result.far_clip = camera["farClip"].toDouble();
    if(camera.contains(QString("focalDistance"))) result.focalD = camera["focalDistance"].toDouble();
    if(camera.contains(QString("lensRadius"))) result.lensR = camera["lensRadius"].toDouble();

    result.RecomputeAttributes();
    return result;
}

Transform JSONReader::LoadTransform(QJsonObject &transform)
{
    Vector3f t, r, s;
    s = Vector3f(1,1,1);
    if(transform.contains(QString("translate"))) t = ToVec3(transform["translate"].toArray());
    if(transform.contains(QString("rotate"))) r = ToVec3(transform["rotate"].toArray());
    if(transform.contains(QString("scale"))) s = ToVec3(transform["scale"].toArray());
    return Transform(t, r, s);
}

glm::vec3 JSONReader::ToVec3(const QJsonArray &s)
{
    glm::vec3 result(s.at(0).toDouble(), s.at(1).toDouble(), s.at(2).toDouble());
    return result;
}

glm::vec3 JSONReader::ToVec3(const QStringRef &s)
{
    glm::vec3 result;
    int start_idx;
    int end_idx = -1;
    for(int i = 0; i < 3; i++){
        start_idx = ++end_idx;
        while(end_idx < s.length() && s.at(end_idx) != QChar(' '))
        {
            end_idx++;
        }
        result[i] = s.mid(start_idx, end_idx - start_idx).toFloat();
    }
    return result;
}
