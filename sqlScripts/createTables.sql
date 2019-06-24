USE WebVisu;
-- Pages
CREATE TABLE IF NOT EXISTS `Pages` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `ParentID` bigint(20) unsigned DEFAULT NULL,
  `title` varchar(30) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `Pages_Pages_FK` (`ParentID`),
  CONSTRAINT `Pages_Pages_FK` FOREIGN KEY (`ParentID`) REFERENCES `Pages` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementTypes
CREATE TABLE `GuiElementTypes` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `type` varchar(32) NOT NULL,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `GuiElementTypes_UN` (`type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElement
CREATE TABLE IF NOT EXISTS `GuiElements` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `PageID` bigint(20) unsigned DEFAULT NULL,
  `TypeID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElement_Pages_FK` (`PageID`),
  KEY `GuiElement_GuiElementTypes_FK` (`TypeID`),
  CONSTRAINT `GuiElement_GuiElementTypes_FK` FOREIGN KEY (`TypeID`) REFERENCES `GuiElementTypes` (`ID`),
  CONSTRAINT `GuiElement_Pages_FK` FOREIGN KEY (`PageID`) REFERENCES `Pages` (`ID`) ON DELETE CASCADE
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
  `GuiElementTypeID` int(10) unsigned NOT NULL,
  `DataNodeTemplateID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GuiElementTypeID`,`DataNodeTemplateID`),
  KEY `GuiElementTypesDataNodesRel_FK_1` (`DataNodeTemplateID`),
  CONSTRAINT `GuiElementTypesDataNodesRel_FK` FOREIGN KEY (`GuiElementTypeID`) REFERENCES `GuiElementTypes` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementTypesDataNodesRel_FK_1` FOREIGN KEY (`DataNodeTemplateID`) REFERENCES `GuiElementDataNodeTemplates` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementDataNodes
CREATE TABLE `GuiElementDataNodes` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `GuiElementID` bigint(20) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `initValue` varchar(100) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementDataNodes_GuiElement_FK` (`GuiElementID`),
  KEY `GuiElementDataNodes_FK` (`typeID`),
  CONSTRAINT `GuiElementDataNodes_FK` FOREIGN KEY (`typeID`) REFERENCES `GuiElementDataNodeTemplates` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementDataNodes_GuiElement_FK` FOREIGN KEY (`GuiElementID`) REFERENCES `GuiElements` (`ID`) ON DELETE CASCADE
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
  `GuiElementTypeID` int(10) unsigned NOT NULL,
  `ParamTemplateID` int(10) unsigned NOT NULL,
  PRIMARY KEY (`GuiElementTypeID`,`ParamTemplateID`),
  KEY `GuiElementTypesParamsRel_FK` (`ParamTemplateID`),
  CONSTRAINT `GuiElementTypesParamsRel_FK` FOREIGN KEY (`ParamTemplateID`) REFERENCES `GuiElementParamTemplates` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementTypesParamsRel_FK_1` FOREIGN KEY (`GuiElementTypeID`) REFERENCES `GuiElementTypes` (`ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- GuiElementParams
CREATE TABLE `GuiElementParams` (
  `ID` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `GuiElementID` bigint(20) unsigned NOT NULL,
  `typeID` int(10) unsigned NOT NULL,
  `value` varchar(100) NOT NULL,
  PRIMARY KEY (`ID`),
  KEY `GuiElementParams_GuiElement_FK` (`GuiElementID`),
  KEY `GuiElementParams_FK` (`typeID`),
  CONSTRAINT `GuiElementParams_FK` FOREIGN KEY (`typeID`) REFERENCES `GuiElementParamTemplates` (`ID`) ON DELETE CASCADE,
  CONSTRAINT `GuiElementParams_GuiElement_FK` FOREIGN KEY (`GuiElementID`) REFERENCES `GuiElements` (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
