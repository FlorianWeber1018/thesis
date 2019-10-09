USE WebVisu;
-- Pages
CREATE TABLE IF NOT EXISTS `Pages` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `parentID` bigint(20) unsigned DEFAULT NULL,
  `title` varchar(30) NOT NULL,
  `description` varchar(250) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `Pages_Pages_FK` (`ParentID`),
  CONSTRAINT `Pages_Pages_FK` FOREIGN KEY (`parentID`) REFERENCES `Pages` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementTypes
CREATE TABLE `GuiElementTypes` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` varchar(32) NOT NULL,
  `description` varchar(250) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `GuiElementTypes_UN` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElement
CREATE TABLE IF NOT EXISTS `GuiElements` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `pageID` bigint(20) unsigned DEFAULT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `name` varchar(30) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElement_Pages_FK` (`pageID`),
  KEY `GuiElement_GuiElementTypes_FK` (`typeID`),
  CONSTRAINT `GuiElement_GuiElementTypes_FK` FOREIGN KEY (`typeID`) REFERENCES `GuiElementTypes` (`ID`),
  CONSTRAINT `GuiElement_Pages_FK` FOREIGN KEY (`pageID`) REFERENCES `Pages` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- DataTypes
CREATE TABLE `DataTypes` (
  `type` varchar(100) NOT NULL,
  PRIMARY KEY (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementDataNodeTemplates
CREATE TABLE `GuiElementDataNodeTemplates` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `writePermission` tinyint(1) DEFAULT false,
  `type` varchar(100) NOT NULL,
  `qualifiedName` varchar(100) DEFAULT 'unnamed',
  `defaultValue` varchar(100) DEFAULT NULL,
  `description` varchar(250) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementDataNodeTemplates_FK` (`type`),
  CONSTRAINT `GuiElementDataNodeTemplates_FK` FOREIGN KEY (`type`) REFERENCES `DataTypes` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementTypesDataNodesRel
CREATE TABLE `GuiElementTypesDataNodesRel` (
  `guiElementTypeID` int(10) unsigned NOT NULL,
  `dataNodeTemplateID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`guiElementTypeID`,`dataNodeTemplateID`),
  KEY `GuiElementTypesDataNodesRel_FK_1` (`dataNodeTemplateID`),
  CONSTRAINT `GuiElementTypesDataNodesRel_FK` FOREIGN KEY (`guiElementTypeID`) REFERENCES `GuiElementTypes` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementTypesDataNodesRel_FK_1` FOREIGN KEY (`dataNodeTemplateID`) REFERENCES `GuiElementDataNodeTemplates` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementDataNodes
CREATE TABLE `GuiElementDataNodes` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `guiElementID` bigint(20) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `initValue` varchar(100) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementDataNodes_GuiElement_FK` (`guiElementID`),
  KEY `GuiElementDataNodes_FK` (`typeID`),
  CONSTRAINT `GuiElementDataNodes_FK` FOREIGN KEY (`typeID`) REFERENCES `GuiElementDataNodeTemplates` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementDataNodes_GuiElement_FK` FOREIGN KEY (`guiElementID`) REFERENCES `GuiElements` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementParamTemplates
CREATE TABLE `GuiElementParamTemplates` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` varchar(100) NOT NULL,
  `qualifiedName` varchar(100) DEFAULT 'unnamed',
  `defaultValue` varchar(100) DEFAULT NULL,
  `description` varchar(250) DEFAULT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementParamTemplates_FK` (`type`),
  CONSTRAINT `GuiElementParamTemplates_FK` FOREIGN KEY (`type`) REFERENCES `DataTypes` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementTypesParamsRel
CREATE TABLE `GuiElementTypesParamsRel` (
  `guiElementTypeID` int(10) unsigned NOT NULL,
  `paramTemplateID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GuiElementTypeID`,`ParamTemplateID`),
  KEY `GuiElementTypesParamsRel_FK` (`paramTemplateID`),
  CONSTRAINT `GuiElementTypesParamsRel_FK` FOREIGN KEY (`paramTemplateID`) REFERENCES `GuiElementParamTemplates` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementTypesParamsRel_FK_1` FOREIGN KEY (`guiElementTypeID`) REFERENCES `GuiElementTypes` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementParams
CREATE TABLE `GuiElementParams` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `guiElementID` bigint(20) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `value` varchar(100) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementParams_GuiElement_FK` (`guiElementID`),
  KEY `GuiElementParams_FK` (`typeID`),
  CONSTRAINT `GuiElementParams_FK` FOREIGN KEY (`typeID`) REFERENCES `GuiElementParamTemplates` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementParams_GuiElement_FK` FOREIGN KEY (`guiElementID`) REFERENCES `GuiElements` (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE `Credentials` (
  userName varchar(100) NOT NULL,
  pwHash varchar(100) NOT NULL,
  PRIMARY KEY (`userName`)
)  ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
