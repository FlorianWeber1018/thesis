DELIMITER $$
CREATE OR REPLACE PROCEDURE CreateInstanceOfGuiElement(
	IN inType varchar(32),
	IN inPageID BIGINT(20) UNSIGNED,
	IN inName varchar(32)
)
BEGIN
	DECLARE typeID TYPE OF GuiElementTypes.ID;
	DECLARE insertedGuiElementID TYPE OF GuiElements.ID;
	set typeID = NULL;
	SELECT DISTINCT ID into typeID from GuiElementTypes WHERE GuiElementTypes.`type` = inType;
	IF(typeID IS Not NULL) THEN
	 	 INSERT INTO GuiElements (GuiElements.PageID, GuiElements.TypeID, GuiElements.name) VALUES (InPageID, typeID, inName);
	 	 set insertedGuiElementID = LAST_INSERT_ID();
	     	 INSERT INTO GuiElementDataNodes (GuiElementID, typeID, initValue)
		 	Select insertedGuiElementID, A.DataNodeTemplateID, B.defaultValue from GuiElementTypesDataNodesRel AS A left Join GuiElementDataNodeTemplates AS B ON B.ID = A.DataNodeTemplateID where GuiElementTypeID = typeID;
		 INSERT INTO GuiElementParams (GuiElementID, typeID, value)
		 	Select insertedGuiElementID, A.ParamTemplateID, B.defaultValue from GuiElementTypesParamsRel AS A left Join GuiElementParamTemplates AS B ON B.ID = A.ParamTemplateID where GuiElementTypeID = typeID;
 	END IF;
END; $$

CREATE OR REPLACE PROCEDURE WebVisu.getDataNodesByGuiElementID(IN inGuiElementID BIGINT(20) UNSIGNED)
BEGIN
	select 
B.`type` as `type`, 
A.initValue as initValue,
B.description as description,
B.qualifiedName as name,
A.guiElementID as parentguiElementSqlID,
A.ID as newDataNodeSqlID,
B.writePermission as writePermission
	from GuiElementDataNodes AS A
LEFT JOIN GuiElementDataNodeTemplates AS B ON B.ID = A.typeID WHERE A.guiElementID = inGuiElementID;
END; $$

CREATE OR REPLACE PROCEDURE WebVisu.getGuiElementsByPageID(IN inPageID BIGINT(20) UNSIGNED)
BEGIN
SELECT
A.name as name,
B.`type` as `type`,
B.description as description,
A.pageID as parentPageSqlID,
A.ID as newGuiELementSqlID
FROM GuiElements A
LEFT JOIN GuiElementTypes AS B ON B.ID = A.typeID
WHERE A.pageID = inPageID;
END; $$

CREATE OR REPLACE PROCEDURE WebVisu.getPagesByParentPageID(IN inParentPageID BIGINT(20) UNSIGNED)
BEGIN
	IF(inParentPageID IS NULL) THEN
		select title, description, parentID as parentPageSqlID, ID as newPageSqlID from Pages where parentID is NULL;
	ELSE
		select title, description, parentID as parentPageSqlID, ID as newPageSqlID from Pages where parentID = inParentPageID;
	END IF;
END; $$


DELIMITER ;
