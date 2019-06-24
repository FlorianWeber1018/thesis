DELIMITER $$
CREATE OR REPLACE PROCEDURE CreateInstanceOfGuiElement(
	IN inType varchar(32),
	IN inPageID BIGINT(20) UNSIGNED
)
BEGIN
	DECLARE typeID TYPE OF GuiElementTypes.ID;
	DECLARE insertedGuiElementID TYPE OF GuiElements.ID;
	set typeID = NULL;
	SELECT DISTINCT ID into typeID from GuiElementTypes WHERE GuiElementTypes.`type` = inType;
	IF(typeID IS Not NULL) THEN
	 	 INSERT INTO GuiElements (GuiElements.PageID, GuiElements.TypeID) VALUES (InPageID, typeID);
	 	 set insertedGuiElementID = LAST_INSERT_ID();
	     	 INSERT INTO GuiElementDataNodes (GuiElementID, typeID, initValue)
		 	Select insertedGuiElementID, A.DataNodeTemplateID, B.defaultValue from GuiElementTypesDataNodesRel AS A left Join GuiElementDataNodeTemplates AS B ON B.ID = A.DataNodeTemplateID where GuiElementTypeID = typeID;
		 INSERT INTO GuiElementParams (GuiElementID, typeID, value)
		 	Select insertedGuiElementID, A.ParamTemplateID, B.defaultValue from GuiElementTypesParamsRel AS A left Join GuiElementParamTemplates AS B ON B.ID = A.ParamTemplateID where GuiElementTypeID = typeID;
 	END IF;
END; $$
DELIMITER ;
